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

#include <cmath>
#include <algorithm>

#include "mmImage.h"

// converts the uv coordinates from uv space ti omage space, doing CLAMP and y flip 
void mapCoordClamped(const glm::vec2& uv, const glm::ivec2& mapSize, glm::ivec2& mapCoord) {
	
	// clamp
	mapCoord.x = (int)(uv[0] * (mapSize.x - 1) + 0.5F); // add 0.5 and cast to int to do round
	mapCoord.x = std::min(std::max(mapCoord.x, 0), mapSize.x - 1);

	// flip the image vertically and clamp
	mapCoord.y = (int)(uv[1] * (mapSize.y - 1) + 0.5F); // add 0.5 and cast to int to do round
	mapCoord.y = mapSize.y - 1 - std::min(std::max(mapCoord.y, 0), mapSize.y - 1);
}

// texture lookup with clamp
void texture2D(const Image& texMap, const glm::vec2& uv, glm::vec3& rgb)
{
	const glm::ivec2 mapSize = { texMap.width, texMap.height };
	glm::ivec2 mapCoord = {0,0};
	mapCoordClamped(uv, mapSize, mapCoord);

	texMap.fetchRGB(mapCoord.x, mapCoord.y, rgb);

}

// this code translated from https ://community.khronos.org/t/manual-bilinear-filter/58504
// rgb is the func output
// fract - compute the fractional part of the argument
// fract(x) = x - floor(x)
// mix — linearly interpolate between two values
// mix (x,y,a) = x * (1 - a) + y * a
void texture2D_bilinear(const Image& texMap, const glm::vec2& uv, glm::vec3& rgb)
{
	const int textureSize[] = { texMap.width, texMap.height };
	glm::vec2 texelSize = { 1.0F / textureSize[0], 1.0F / textureSize[1] };

	// vec2 f = fract( uv * textureSize );
	glm::vec2 uv_scaled = { uv[0] * textureSize[0], uv[1] * textureSize[1] };
	glm::vec2 f = { uv_scaled[0] - std::floor(uv_scaled[0]), uv_scaled[1] - std::floor(uv_scaled[1]) };

	// move uv to texel centre
	// uv += ( .5 - f ) * texelSize;
	glm::vec2 uv_cntr = { (.5F - f[0]) * texelSize[0], (.5F - f[0]) * texelSize[0] };

	//vec4 tl = texture2D(t, uv);
	//vec4 tr = texture2D(t, uv + vec2(texelSize.x, 0.0));
	//vec4 bl = texture2D(t, uv + vec2(0.0, texelSize.y));
	//vec4 br = texture2D(t, uv + vec2(texelSize.x, texelSize.y));
	glm::vec2 uvtr = { uv[0] + texelSize[0], uv[1] };
	glm::vec2 uvbl = { uv[0], uv[1] + texelSize[1] };
	glm::vec2 uvbr = { uv[0] + texelSize[0], uv[1] + texelSize[1] };
	glm::vec3 tl, tr, bl, br;
	texture2D(texMap, uv, tl);
	texture2D(texMap, uvtr, tr);
	texture2D(texMap, uvbl, bl);
	texture2D(texMap, uvbr, br);
	//
	// vec4 tA = mix( tl, tr, f.x );
	// vec4 tB = mix( bl, br, f.x );
	float ta[3] = {
		   tl[0] * (1.0F - f[0]) + tr[0] * f[0],
		   tl[1] * (1.0F - f[0]) + tr[1] * f[0],
		   tl[2] * (1.0F - f[0]) + tr[2] * f[0]
	};
	float tb[3] = {
		   bl[0] * (1.0F - f[0]) + br[0] * f[0],
		   bl[1] * (1.0F - f[0]) + br[1] * f[0],
		   bl[2] * (1.0F - f[0]) + br[2] * f[0]
	};
	// return mix( tA, tB, f.y );
	rgb[0] = ta[0] * (1.0F - f[1]) + tb[0] * f[1];
	rgb[1] = ta[1] * (1.0F - f[1]) + tb[1] * f[1];
	rgb[2] = ta[2] * (1.0F - f[1]) + tb[2] * f[1];
}