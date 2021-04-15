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

#ifndef _MM_IMAGE_H_
#define _MM_IMAGE_H_

#include "glm/glm.hpp"

//
class Image {
public:
	int width;
	int height;
	int nbc; // # 8-bit component per pixel
	unsigned char* data;

	Image(void):width(0),height(0),nbc(0),data(NULL){}

	// no sanity check for performance reasons
	inline void fetchRGB(const size_t col, const size_t row,  glm::vec3& rgb) const {
		rgb.r = data[(row * width + col) * nbc + 0];
		rgb.g = data[(row * width + col) * nbc + 1];
		rgb.b = data[(row * width + col) * nbc + 2];
	}

};

// converts the uv coordinates from uv space to image space, doing CLAMP and y flip 
void mapCoordClamped(const glm::vec2& uv, const glm::ivec2& mapSize, glm::ivec2& mapCoord);

// texture lookup with clamp
void texture2D(const Image& tex_map, const glm::vec2& uv, glm::vec3& rgb);

// texture lookup with bilinear filtering
void texture2D_bilinear(const Image& tex_map, const glm::vec2& uv, glm::vec3& rgb);

#endif