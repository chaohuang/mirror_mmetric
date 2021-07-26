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

#ifndef _MM_COLOR_SPACES_H_
#define _MM_COLOR_SPACES_H_

// mathematics
#include <glm/vec3.hpp>

// converts yuv or rgb colors from [0,255] to [0,1]
inline void color256ToUnit(const glm::vec3& in256, glm::vec3& outUnit) {
	const float divider = 1.0F / 255.0F;
	// shall be vectorized by compiler
	for ( glm::vec3::length_type i = 0; i < 3 ; ++i )
		outUnit[i] = in256[i] * divider;
}

// converts yuv or rgb colors from [0,1] to [0,255]
inline void colorUnitTo256(const glm::vec3& in256, glm::vec3& outUnit) {
	// shall be vectorized by compiler
	for ( glm::vec3::length_type i = 0; i < 3; ++i )
		outUnit[i] = in256[i] * 255.0F;
}

// RGB and YUV components in [0,255]
inline void rgbToYuvBt709_256(const glm::vec3& rgb, glm::vec3& yuv) {

	yuv[0] = 0.2126F  * rgb[0] + 0.7152F * rgb[1] + 0.0722F * rgb[2];
	yuv[1] = -0.1146F * rgb[0] - 0.3854F * rgb[1] + 0.5000F * rgb[2] + 128.0F;
	yuv[2] = 0.5000F  * rgb[0] - 0.4542F * rgb[1] - 0.0458F * rgb[2] + 128.0F;
}

// RGB and YUV components in [0,255]
inline glm::vec3 rgbToYuvBt709_256(const glm::vec3& rgb) {

	return glm::vec3(
		0.2126F  * rgb[0] + 0.7152F * rgb[1] + 0.0722F * rgb[2],
		-0.1146F * rgb[0] - 0.3854F * rgb[1] + 0.5000F * rgb[2] + 128.0F,
		0.5000F  * rgb[0] - 0.4542F * rgb[1] - 0.0458F * rgb[2] + 128.0F );
}

// RGB and YUV components in [0,255]
inline void yuvBt709ToRgb_256(const glm::vec3& yuv, glm::vec3& rgb) {

	const float uc = yuv[1] - 128.0F;
	const float vc = yuv[2] - 128.0F;

	rgb[0] = yuv[0] + 1.57480F * vc;
	rgb[1] = yuv[0] - 0.18733F * uc - 0.46813F * vc;
	rgb[2] = yuv[0] + 1.85563F * uc;

}

// RGB and YUV components in [0,1]
inline void rgbToYuvBt709_unit(const glm::vec3& rgb, glm::vec3& yuv) {

	yuv[0] = 0.2126F  * rgb[0] + 0.7152F * rgb[1] + 0.0722F * rgb[2];
	yuv[1] = -0.1146F * rgb[0] - 0.3854F * rgb[1] + 0.5000F * rgb[2] + 0.5F;
	yuv[2] = 0.5000F  * rgb[0] - 0.4542F * rgb[1] - 0.0458F * rgb[2] + 0.5F;
}

// RGB and YUV components in [0,1]
inline void yuvBt709ToRgb_unit(const glm::vec3& yuv, glm::vec3& rgb) {

	const float uc = yuv[1] - 0.5F;
	const float vc = yuv[2] - 0.5F;

	rgb[0] = yuv[0] + 1.57480F * vc;
	rgb[1] = yuv[0] - 0.18733F * uc - 0.46813F * vc;
	rgb[2] = yuv[0] + 1.85563F * uc;

}

#endif
