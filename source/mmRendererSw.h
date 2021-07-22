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

#ifndef _MM_RENDERER_SW_H_
#define _MM_RENDERER_SW_H_

#include <string>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

// internal headers
#include "mmImage.h"
#include "mmModel.h"

struct RendererSw {

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
		const std::string& outputImage, const std::string& outputDepth,
		const unsigned int width, const unsigned int height,
		const glm::vec3& viewDir, const glm::vec3& viewUp,
		const glm::vec3& bboxMin, const glm::vec3& bboxMax, bool useBBox);

	// Buffers cleanup
	void clear(std::vector<uint8_t>& fbuffer, std::vector<float>& zbuffer);
	inline void setClearColor(glm::vec4 color) { _clearColor = color; }
	// depth shall be negative
	inline void setClearDepth(float depth) { _clearDepth = depth; } 
	
	// Culling
	inline void enableCulling() { _isCullingEnabled = true; }
	inline void disableCulling() { _isCullingEnabled = false; }
	// if true sets to CW culling, CCW otherwise
	inline void setCwCulling( bool value ) { _cwCulling = value; }

	// Lighting
	inline void enableLighting() { _isLigthingEnabled = true; }
	inline void disableLighting() { _isLigthingEnabled = false; }
	inline void enableAutoLightPosition() { _isAutoLightPositionEnabled = true; }
	inline void disableAutoLightPosition() { _isAutoLightPositionEnabled = false; }
	inline void setLightAutoDir(glm::vec3 direction) { _lightAutoDir = direction; }
	inline void setLightPosition(glm::vec4 position) { _lightPosition = position; }
	inline void setLightcolor(glm::vec3 color) { _lightColor = color; }
	
	// Materials
	inline void setMaterialAmbient(glm::vec3 Ka) { _materialAmbient = Ka; }
	inline void setMaterialDiffuse(glm::vec3 Kd) { _materialDiffuse = Kd; }
	
	// Post process
	inline void enableAutoLevel() { _isAutoLevelEnabled = true; }
	inline void disableAutoLevel() { _isAutoLevelEnabled = false; }

public:
	// after each render this values are updated and can be read
	float depthRange; // represents the maximum depth for the render bbox in the depthBuffer

private:

	glm::vec4 _clearColor{ 0, 0, 0, 0 };
	float _clearDepth = -std::numeric_limits<float>::max();

	bool _isCullingEnabled = false;	// enable back face culling
	bool _cwCulling = true;			// defaults faces orientation to clock wise culling

	bool _isLigthingEnabled = false;
	bool _isAutoLightPositionEnabled = false;
	glm::vec3 _lightAutoDir{ 1.0F,1.0F,1.0F };       // vector to compute automatic position, top right by default
	glm::vec4 _lightPosition{ 0, 0, 0, 1 };	         // user defined position
	glm::vec3 _lightColor{ 1.0F, 1.0F, 1.0F };       // in 0-1 for each component

	glm::vec3 _materialAmbient{ 0.4F, 0.4F, 0.4F };  // Kd for each component
	glm::vec3 _materialDiffuse{ 0.6F, 0.6F, 0.6F };  // Ka for each component

	bool _isAutoLevelEnabled = false;

};

#endif
