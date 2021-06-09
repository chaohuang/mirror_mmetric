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

#ifndef _MM_RENDER_H_
#define _MM_RENDER_H_

// internal headers
#include "mmCommand.h"
#include "mmModel.h"
#include "mmImage.h"
#include "mmRendererHw.h"
#include "mmRendererSw.h"

class Render : Command {

private:

	// the command options
	std::string inputModelFilename;
	std::string inputTextureFilename;
	std::string outputImageFilename = "output.png";
	std::string outputDepthFilename = "";
	std::string renderer = "sw_raster";
	bool hideProgress = false;
	// the type of processing
	unsigned int width = 1920;
	unsigned int height = 1080;
	// filtering bilinear or nearest
	bool bilinear = true;
	// view vectors
	std::string viewDirStr;
	std::string viewUpStr;
	glm::vec3 viewDir = { 0.0F, 0.0F, 1.0F };
	glm::vec3 viewUp = { 0.0F, 1.0F, 0.0F };
	// bbox min and max
	std::string bboxMinStr;
	std::string bboxMaxStr;
	glm::vec3 bboxMin;
	glm::vec3 bboxMax;
	bool bboxValid = false;
	// clear color
	std::string clearColorStr;
	glm::vec4 clearColor = { 0, 0, 0, 0 };
	// culling
	bool enableCulling = false;
	bool cwCulling = true;
	// lighting
	bool enableLighting=false;
	bool autoLightPosition=false;
	std::string lightAutoDirStr;
	glm::vec3 lightAutoDir{ 1.0F, 1.0F, 1.0F };
	// the Software renderer
	RendererSw _swRenderer;
	// the Hardware renderer
	RendererHw _hwRenderer;

public:

	Render() {};

	// Description of the command
	static const char* name;
	static const char* brief;
	// command creator
	static Command* create();

	// the command main program
	virtual bool initialize(Context* ctx, std::string app, int argc, char* argv[]);
	virtual bool process(uint32_t frame);
	virtual bool finalize();

};

#endif