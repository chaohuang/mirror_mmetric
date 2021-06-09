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

#ifndef _MM_RENDERER_HW_H_
#define _MM_RENDERER_HW_H_

#include <string>

//
#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

// internal headers
#include "mmImage.h"
#include "mmModel.h"

struct RendererHw {

	// open the output render context and associated hidden window
	bool initialize( const unsigned int width, const unsigned int height );
	// release OpenGL resources
	bool shutdown(void);

	// render a mesh to memory
	bool render(
		Model* model, const Image* map,
		std::vector<uint8_t>& fbuffer,
		std::vector<float>& zbuffer,
		const unsigned int width, const unsigned int height,
		const glm::vec3& viewDir, const glm::vec3& viewUp,
		const glm::vec3& bboxMin, const glm::vec3& bboxMax, bool useBBox);

	// render a mesh to PNG image files
	bool render(
		Model* model, const Image* map,
		const std::string& outputImage, 
		const std::string& outputDepth,
		const unsigned int width, const unsigned int height,
		const glm::vec3& viewDir, const glm::vec3& viewUp,
		const glm::vec3& bboxMin, const glm::vec3& bboxMax, bool useBBox);

	// clear the buffers
	void clear(std::vector<char>& fbuffer, std::vector<float>& zbuffer);

	//
	inline void setClearColor(glm::vec4 color) { _clearColor = color; }

	// Culling
	inline void enableCulling() { _isCullingEnabled = true; }
	inline void disableCulling() { _isCullingEnabled = false; }
	// if true sets to CW culling, CCW otherwise
	inline void setCwCulling(bool value) { _cwCulling = value; }

private:

	// render parameters
	glm::vec4 _clearColor{ 0, 0, 0, 0 };

	bool _isCullingEnabled = false;	// enable back face culling
	bool _cwCulling = true;			// defaults faces orientation to clock wise culling

	// GLFW 
	GLFWwindow* _window;
	unsigned int _width = 640;
	unsigned int _height = 480;

	// GL stuffs
	GLuint _fbo, _color_rb, _depth_rb;
	
};

#endif
