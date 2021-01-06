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

};

// texture lookup with clamp
void texture2D(const Image& tex_map, const glm::vec2& uv, glm::vec3& rgb);

// texture lookup with bilinear filtering
void texture2D_bilinear(const Image& tex_map, const glm::vec2& uv, glm::vec3& rgb);

#endif