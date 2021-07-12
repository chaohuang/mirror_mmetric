// ************* COPYRIGHT AND CONFIDENTIALITY INFORMATION *********
// Copyright 2021 - InterDigital
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// 
// http ://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissionsand
// limitations under the License.
//
// Author: jean-eudes.marvie@interdigital.com
// *****************************************************************

#ifndef _MM_COMPARE_H_
#define _MM_COMPARE_H_

// internal headers
#include "mmCommand.h"
#include "mmModel.h"
// MPEG PCC metric
#include "pcc/pcc_distortion.hpp"
//
#include "mmRendererHw.h"
#include "mmRendererSw.h"

class Compare : Command {

public:

	struct IbsmResults {
		double rgbMSE[4]  = { 0,0,0,0 }; // [R mse, G mse, B mse, components mean]
		double rgbPSNR[4] = { 0,0,0,0 }; // idem but with psnr
		double yuvMSE[4]  = { 0,0,0 };   // [Y mse, U mse, V mse, 6/1/1 mean ]
		double yuvPSNR[4] = { 0,0,0 };   // idem but with psnr
		double depthMSE = 0;
		double depthPSNR = 0;
	};

private:

	// the context for frame access
	Context* _context;
	// the command options
	std::string _inputModelAFilename, _inputModelBFilename;
	std::string _inputTextureAFilename, _inputTextureBFilename;
	std::string _outputModelAFilename, _outputModelBFilename;
	std::string _outputCsvFilename;
	// the type of processing
	std::string _mode = "equ";
	// Equ options
	float _equEpsilon = 0;
	bool _equEarlyReturn = true;
	bool _equUnoriented = false;
	// Topo options
	std::string _topoFaceMapFilename;
	std::string _topoVertexMapFilename;
	// Pcc options
	pcc_quality::commandPar _pccParams;
	// PCQM options
	double _pcqmRadiusCurvature = 0.001;
	int _pcqmThresholdKnnSearch = 20;
	double _pcqmRadiusFactor = 2.0;
	// Pcc results array of <frame, result>
	std::vector < std::pair<uint32_t, pcc_quality::qMetric> > _pccResults;
	// PCQM results array of <frame, pcqm, pcqm-psnr>
	std::vector < std::tuple<uint32_t, double, double > > _pcqmResults;
	// Raster results array of <frame, result>
	std::vector < std::pair<uint32_t, IbsmResults> > _ibsmResults;

	// Raster options
	unsigned int _ibsmResolution = 2048;
	unsigned int _ibsmCameraCount = 16;
	std::string _ibsmCameraRotation = "0.0 0.0 0.0";
	glm::vec3 CamRotParams = { 0.0F, 0.0F, 0.0F };
	std::string _ibsmRenderer = "sw_raster";
	std::string _ibsmOutputPrefix = "";
	bool _ibsmDisableReordering = false;
	bool _ibsmDisableCulling = false;

	// Renderers for the ibsm metric
	RendererSw _swRenderer; // the Software renderer
	RendererHw _hwRenderer; // the Hardware renderer

public:

	Compare() {
		_pccParams.singlePass = false;
		_pccParams.hausdorff = false;
		_pccParams.bColor = true;
		_pccParams.bLidar = false; // allways false, no option
		_pccParams.resolution = 0.0; // auto
		_pccParams.neighborsProc = 1;
		_pccParams.dropDuplicates = 2;
		_pccParams.bAverageNormals = true;
	};

	// Descriptions of the command
	static const char* name;
	static const char* brief;

	// command creator
	static Command* create();

	// the command main program
	virtual bool initialize(Context* ctx, std::string app, int argc, char* argv[]);
	virtual bool process(uint32_t frame);
	virtual bool finalize();

	// compare two meshes for equality (using mem comp if epsilon = 0)
	// if epsilon = 0, return 0 on success and 1 on difference
	// if epsilon > 0, return 0 on success and nb diff on difference if sizes are equal, 1 otherwise
	int equ(
		const Model& modelA, const Model& modelB,
		const Image& mapA, const Image& mapB,
		float epsilon, bool earlyReturn, bool unoriented,
		Model& outputA, Model& outputB);

	// compare two meshes topology for equivalence up to face index shift
	// check topology will use a bijective face map, associating output triangles to input triangles:
	// - faceMap file shall contain the association dest face index -> orig face index for each face, one face per line
	// - %d in filename is to be resolved before invocation
	// check topology will use a bijective vartex map, associating output vertices to input vertices:
	// - vertexMap file shall contain the association dest vertex index -> orig vertex index for each vertex, one vertex per line
	// - %d in filename is to be resolved before invocation
	// the function validates the following points:
	// - Test if number of triangles of output matches input number of triangles
	// - Test if the proposed association tables for face and vertex are bijective
	// - Test if each output triangle respects the orientation of its associated input triangle
	int topo(
		const Model& modelA, const Model& modelB,
		const std::string& faceMapFilenane = "",
		const std::string& vertexMapFilenane = "");

	// compare two meshes using MPEG pcc_distortion metric
	int pcc(
		const Model& modelA, const Model& modelB,
		const Image& mapA, const Image& mapB,
		pcc_quality::commandPar& params,
		Model& outputA, Model& outputB
	);
	
	// collect statics over sequence and compute results
	void pccFinalize(void);

	// compare two meshes using PCQM metric
	int pcqm(
		const Model& modelA, const Model& modelB,
		const Image& mapA, const Image& mapB,
		const double radiusCurvature,
		const int thresholdKnnSearch,
		const double radiusFactor,
		Model& outputA, Model& outputB
	);

	// collect statics over sequence and compute results
	void pcqmFinalize(void);

	// compare two meshes using rasterization
	int ibsm(
		const Model& modelA, const Model& modelB,
		const Image& mapA, const Image& mapB,
		Model& outputA, Model& outputB
	);

	// collect statics over sequence and compute results
	void ibsmFinalize(void);

};

#endif
