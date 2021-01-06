// ************* COPYRIGHT AND CONFIDENTIALITY INFORMATION *********
// Copyright © 20XX InterDigital All Rights Reserved
// This program contains proprietary information which is a trade secret/business
// secret of InterDigital R&D france is protected, even if unpublished, under 
// applicable Copyright laws (including French droit d’auteur) and/or may be 
// subject to one or more patent(s).
// Recipient is to retain this program in confidence and is not permitted to use 
// or make copies thereof other than as permitted in a written agreement with 
// InterDigital unless otherwise expressly allowed by applicable laws or by 
// InterDigital under express agreement.
//
// Author: jean-eudes.marvie@interdigital.com
// *****************************************************************

#include <cmath>
#include <algorithm>

#include "mmImage.h"

// texture lookup with clamp
void texture2D(const Image& tex_map, const glm::vec2& uv, glm::vec3& rgb)
{
	int textureSize[] = { tex_map.width, tex_map.height };

	// clamp
	int x = (int)round(uv[0] * textureSize[0] - 1);
	x = std::min(std::max(x, 0), textureSize[0] - 1);

	// flip the image vertically and clamp
	int y = (int)round(uv[1] * textureSize[1] - 1);
	y = textureSize[1] - 1 - std::min(std::max(y, 0), textureSize[1] - 1);

	for (int i = 0; i < 3; i++)
		rgb[i] = (float)(tex_map.data[(y * textureSize[0] + x) * tex_map.nbc + i]);
}

// this code translated from https ://community.khronos.org/t/manual-bilinear-filter/58504
// rgb is the func output
// fract - compute the fractional part of the argument
// fract(x) = x - floor(x)
// mix — linearly interpolate between two values
// mix (x,y,a) = x * (1 - a) + y * a
void texture2D_bilinear(const Image& tex_map, const glm::vec2& uv, glm::vec3& rgb)
{
	int textureSize[] = { tex_map.width, tex_map.height };
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
	texture2D(tex_map, uv, tl);
	texture2D(tex_map, uvtr, tr);
	texture2D(tex_map, uvbl, bl);
	texture2D(tex_map, uvbr, br);
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