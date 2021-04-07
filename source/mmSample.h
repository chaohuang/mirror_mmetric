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

	// the command options
	std::string inputModelFilename;
	std::string inputTextureFilename;
	std::string outputModelFilename;
	bool hideProgress = false;
	// the type of processing
	std::string mode = "face";
	// Face options
	float thickness = 0.0;
	// Grid options
	int gridSize = 1024;
	// Face, Grid and sdiv options
	bool bilinear = false;
	// Face subdiv options
	float areaThreshold = 1.0F;
	bool mapThreshold = false;
	// Edge subdiv options (0.0 mean use resolution)
	float lengthThreshold = 0.0F;
	// Edge and Face options
	size_t resolution = 1024;

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
	static void meshToPcFace(const Model& input, Model& output,	const Image& tex_map, 
		size_t resolution, float thickness, bool bilinear, bool logProgress);

	// will sample the mesh on a grid basis of resolution gridRes, result will be generated as float or integer
	static void meshToPcGrid(const Model& input, Model& output,	const Image& tex_map, 
		size_t gridSize, bool bilinear, bool logProgress);

	// revert sampling, guided by texture map
	static void meshToPcMap(const Model& input, Model& output, const Image& tex_map, bool logProgress);

	// triangle dubdivision based, area stop criterion
	static void meshToPcDiv(const Model& input, Model& output, const Image& tex_map, 
		float areaThreshold, bool mapThreshold, bool bilinear, bool logProgress);

	// triangle dubdivision based, edge stop criterion
	static void meshToPcDivEdge(const Model& input, Model& output, const Image& tex_map,
		float lengthThreshold, size_t resolution, bool bilinear, bool logProgress);
};

#endif