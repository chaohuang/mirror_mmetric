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

#include <cmath>
#include <algorithm>

#include "glm/glm.hpp"

//
class Image {
public:
	int width;
	int height;
	int nbc; // # 8-bit component per pixel
	unsigned char* data;

	Image(void):width(0),height(0),nbc(0),data(NULL){}

	~Image(void) {
		delete[] data;
	}

	// no sanity check for performance reasons
	inline void fetchRGB(const size_t col, const size_t row,  glm::vec3& rgb) const {
		rgb.r = data[(row * width + col) * nbc + 0];
		rgb.g = data[(row * width + col) * nbc + 1];
		rgb.b = data[(row * width + col) * nbc + 2];
	}

};

// clamp the map i,j. j is flipped. mapCoord expressed in image space.
inline void clampCoords(const glm::ivec2& mapCoord, const glm::ivec2& mapSize, glm::ivec2& clampedCoord) {

	// clamp
	clampedCoord.x = std::min(std::max(mapCoord.x, 0), mapSize.x - 1);
	// flip the image vertically and clamp
	clampedCoord.y = mapSize.y - 1 - std::min(std::max(mapCoord.y, 0), mapSize.y - 1);

}

// converts the uv coordinates from uv space to image space, doing CLAMP and v flip 
inline void mapCoordClamped(const glm::vec2& uv, const glm::ivec2& mapSize, glm::ivec2& mapCoord) {

	// change from uv space to image space
	const glm::ivec2 coords(
		(int)(uv[0] * mapSize.x),
		(int)(uv[1] * mapSize.y));

	//
	clampCoords(coords, mapSize, mapCoord);
}

// texture lookup using ij in image space with clamp and j flip
inline void image2D(const Image& texMap, const glm::ivec2& ij, glm::vec3& rgb)
{
	const glm::ivec2 mapSize = { texMap.width, texMap.height };
	glm::ivec2 mapCoord = { 0,0 };
	clampCoords(ij, mapSize, mapCoord);
	texMap.fetchRGB(mapCoord.x, mapCoord.y, rgb);
}

// texture lookup using uv (in uv space) with clamp and v flip
inline void texture2D(const Image& texMap, const glm::vec2& uv, glm::vec3& rgb)
{
	const glm::ivec2 mapSize(texMap.width, texMap.height);
	glm::ivec2 mapCoord(0, 0);
	mapCoordClamped(uv, mapSize, mapCoord);
	texMap.fetchRGB(mapCoord.x, mapCoord.y, rgb);
}

// texture lookup with bilinear filtering and v flip
// rgb is the func output
inline void texture2D_bilinear(const Image& texMap, const glm::vec2& uv, glm::vec3& rgb)
{
	const glm::vec2 textureSize(texMap.width, texMap.height);

	// compute sliding window top left pixel pos
	const glm::vec2 pos = uv * textureSize - glm::vec2(0.5);
	const glm::ivec2 posTopLeft = glm::floor(pos);
	// fractional part gives us the blending coeficients
	const glm::vec2 f = fract(pos);

	// extract the pixels
	glm::vec3 tl, tr, bl, br;
	image2D(texMap, posTopLeft, tl);
	image2D(texMap, posTopLeft + glm::ivec2(1, 0), tr);
	image2D(texMap, posTopLeft + glm::ivec2(0, 1), bl);
	image2D(texMap, posTopLeft + glm::ivec2(1, 1), br);

	// put the fruits in the blender, press start
	const glm::vec3 tA = glm::mix(tl, tr, f.x);
	const glm::vec3 tB = glm::mix(bl, br, f.x);
	rgb = glm::mix(tA, tB, f.y);
}

#endif