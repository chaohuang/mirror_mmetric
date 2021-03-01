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

#ifndef _MM_DEQUANTIZE_H_
#define _MM_DEQUANTIZE_H_

// internal headers
#include "mmCommand.h"
#include "mmModel.h"

class Dequantize : Command {

public:

	Dequantize() {};

	// Description of the command
	static const char* name;
	static const char* brief;
	// command creator
	static Command* create();

	// the command main program
	virtual bool initialize(Context* ctx, std::string app, int argc, char* argv[]);
	virtual bool process(uint32_t frame);
	virtual bool finalize() { return true; };

	static void dequantize(
		const Model& input, Model& output,
		const uint32_t qp, const uint32_t qt, const uint32_t qn, const uint32_t qc,
		const glm::vec3& minPos, const glm::vec3& maxPos, const glm::vec2& minUv, const glm::vec2& maxUv,
		const glm::vec3& minNrm, const glm::vec3& maxNrm, const glm::vec3& minCol, const glm::vec3& maxCol);

private:

	// Command parameters
	std::string inputModelFilename;
	std::string outputModelFilename;
	// Quantization parameters
	uint32_t qp = 0; // geometry
	uint32_t qt = 0; // UV coordinates
	uint32_t qn = 0; // normals
	uint32_t qc = 0; // colors

	// min max vectors
	std::string minPosStr;
	std::string maxPosStr;
	glm::vec3 minPos = { 0.0F,0.0F,0.0F };
	glm::vec3 maxPos = { 0.0F,0.0F,0.0F };
	std::string minUvStr;
	std::string maxUvStr;
	glm::vec2 minUv = { 0.0F,0.0F };
	glm::vec2 maxUv = { 0.0F,0.0F };
	std::string minNrmStr;
	std::string maxNrmStr;
	glm::vec3 minNrm = { 0.0F,0.0F,0.0F };
	glm::vec3 maxNrm = { 0.0F,0.0F,0.0F };
	std::string minColStr;
	std::string maxColStr;
	glm::vec3 minCol = { 0.0F,0.0F,0.0F };
	glm::vec3 maxCol = { 0.0F,0.0F,0.0F };

};

#endif
