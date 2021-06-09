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

#ifndef _MM_SAMPLE_H_
#define _MM_SAMPLE_H_

// internal headers
#include "mmCommand.h"
#include "mmModel.h"
#include "mmImage.h"

class Sample : Command {

private:
	// the context for frame access
	Context* _context;
	// the command options
	std::string inputModelFilename;
	std::string inputTextureFilename;
	std::string outputModelFilename;
	std::string _outputCsvFilename;
	bool hideProgress = false;
	// the type of processing
	std::string mode = "face";
	// Face options
	float thickness = 0.0;
	// Grid options
	int _gridSize = 1024;
	bool _useNormal = false;
	bool _useFixedPoint = false;	
	std::string _minPosStr;
	std::string _maxPosStr;
	glm::vec3 _minPos = { 0.0F,0.0F,0.0F };
	glm::vec3 _maxPos = { 0.0F,0.0F,0.0F };
	// Face, Grid and sdiv options
	bool bilinear = false;
	// Face subdiv options
	float areaThreshold = 1.0F;
	bool mapThreshold = false;
	// Edge subdiv options (0.0 mean use resolution)
	float lengthThreshold = 0.0F;
	// Edge and Face options
	size_t _resolution = 1024;
	// sample count constrained sampling
	size_t _nbSamplesMin = 0;
	size_t _nbSamplesMax = 0;
	size_t _maxIterations = 10;
	// Prnd options
	size_t _nbSamples = 2000000;

public:

	Sample() {};

	// Description of the command
	static const char* name;
	static const char* brief;
	// command creator
	static Command* create();

	// the command main program
	virtual bool initialize(Context* ctx, std::string app, int argc, char* argv[]);
	virtual bool process(uint32_t frame);
	virtual bool finalize() { return true; }

	// sample the mesh on a face basis
	static void meshToPcFace(const Model& input, Model& output, const Image& tex_map,
		size_t resolution, float thickness, bool bilinear, bool logProgress);

	// sample the mesh on a face basis
	// system will search the resolution according to the nbSamplesMin and nbSamplesMax parameters
	// costly method, shall be used only for calibration
	static void meshToPcFace(
		const Model& input, Model& output, const Image& tex_map,
		size_t nbSamplesMin, size_t nbSamplesMax, size_t maxIterations,
		float thickness, bool bilinear, bool logProgress, size_t& computedResolution);

	// will sample the mesh on a grid basis of resolution gridRes
	static void meshToPcGrid(const Model& input, Model& output, const Image& tex_map,
		size_t gridSize, bool bilinear, bool logProgress, bool useNormal, 
		bool useFixedPoint,	glm::vec3& minPos, glm::vec3& maxPos);

	// will sample the mesh on a grid basis of resolution gridRes, result will be generated as float or integer
	// system will search the resolution according to the nbSamplesMin and nbSamplesMax parameters
	// costly method, shall be used only for calibration
	static void meshToPcGrid(
		const Model& input, Model& output, const Image& tex_map,
		size_t nbSamplesMin, size_t nbSamplesMax, size_t maxIterations,
		bool bilinear, bool logProgress, bool useNormal,
		bool useFixedPoint, glm::vec3& minPos, glm::vec3& maxPos, size_t& computedResolution);

	// revert sampling, guided by texture map
	static void meshToPcMap(const Model& input, Model& output, const Image& tex_map, bool logProgress);

	// triangle dubdivision based, area stop criterion
	static void meshToPcDiv(const Model& input, Model& output, const Image& tex_map,
		float areaThreshold, bool mapThreshold, bool bilinear, bool logProgress);

	// triangle dubdivision based, area stop criterion
	// system will search the resolution according to the nbSamplesMin and nbSamplesMax parameters
	// costly method, shall be used only for calibration
	static void meshToPcDiv(const Model& input, Model& output, const Image& tex_map,
		size_t nbSamplesMin, size_t nbSamplesMax, size_t maxIterations,
		bool bilinear, bool logProgress, float& computedThres);

	// triangle dubdivision based, edge stop criterion
	static void meshToPcDivEdge(const Model& input, Model& output, const Image& tex_map,
		float lengthThreshold, size_t resolution, bool bilinear, bool logProgress, float& computedThres);

	// triangle dubdivision based, edge stop criterion
	// system will search the resolution according to the nbSamplesMin and nbSamplesMax parameters
	// costly method, shall be used only for calibration
	static void meshToPcDivEdge(const Model& input, Model& output, const Image& tex_map,
		size_t nbSamplesMin, size_t nbSamplesMax, size_t maxIterations,
		bool bilinear, bool logProgress, float& computedThres);

	// pseudo random sampling with point targetPointCount stop criterion
	static void meshToPcPrnd(const Model& input, Model& output, const Image& tex_map,
		size_t targetPointCount, bool bilinear, bool logProgress);

};

#endif
