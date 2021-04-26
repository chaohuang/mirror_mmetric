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

class Compare : Command {
	
private:

	// the context for frame access
	Context* _context;
	// the command options
	std::string inputModelAFilename, inputModelBFilename;
	std::string inputTextureAFilename, inputTextureBFilename;
	std::string outputModelAFilename, outputModelBFilename;
	// the type of processing
	std::string mode = "equ";
	// Equ options
	float epsilon = 0;
	bool earlyReturn = true;
	bool unoriented = false;
	// PCC and PCQM common options
	std::string faceMapFilename;
	std::string vertexMapFilename;
	// Pcc options
	pcc_quality::commandPar params;
	// PCQM options
	double radiusCurvature = 0.001;
	int thresholdKnnSearch = 20;
	double radiusFactor = 2.0;
	// Pcc results array of <frame, result>
	std::vector < std::pair<uint32_t, pcc_quality::qMetric> > pccResults;
	// PCQM results array of <frame, pcqm, pcqm-psnr>
	std::vector < std::tuple<uint32_t, double, double > > pcqmResults;

public:

	Compare() {
		params.singlePass = false;
		params.hausdorff = false;
		params.bColor = true;
		params.bLidar = false; // allways false, no option
		params.resolution = 0.0; // auto
		params.neighborsProc = 1;
		params.dropDuplicates = 2;
		params.bAverageNormals = true;
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
	// check topology will use a bijective vartex map, associating output vertices to input vertices:
	// - vertexMap file shall contain the association dest vertex index -> orig vertex index for each vertex, one vertex per line
	// the function validates the following points:
	// - Test if number of triangles of output matches input number of triangles
	// - Test if the proposed association tables for face and vertex are bijective
	// - Test if each output triangle respects the orientation of its associated input triangle
	int topo(
		const Model& modelA, const Model& modelB,
		const std::string& faceMapFilenane = "",
		const std::string& vertexMapFilenane = "");

	// compare two meshes using MPEG pcc_distortion metric
	// if topoFilename!="" will also check topo using Compare::checkTopology
	int pcc(
		const Model& modelA, const Model& modelB,
		const Image& mapA, const Image& mapB,
		pcc_quality::commandPar& params,
		Model& outputA, Model& outputB
	);
	
	// collect statics over sequence and compute results
	void pccFinalize(void);

	// compare two meshes using PCQM metric
	// if topoFilename!="" will also check topo using Compare::checkTopology
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

};

#endif
