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

#ifndef _MM_SAMPLE_H_
#define _MM_SAMPLE_H_

// internal headers
#include "mmModel.h"
#include "mmImage.h"

struct Sample {
	
	// Descriptions of the commands
	static const char* brief;

	// the command main program
	static int main(std::string app, std::string cmd, int argc, char* argv[]);

	// sample the mesh on a face basis
	static void meshToPcFace(const Model& input, Model& output,	const Image& tex_map, 
		size_t resolution, float thickness, bool bilinear, bool logProgress);

	// will sample the mesh on a grid basis of resolution gridRes, result will be generated as float or integer
	static void meshToPcGrid(const Model& input, Model& output,	const Image& tex_map, 
		size_t gridSize, bool bilinear, bool logProgress);

	// revert sampling, guided by texture map
	static void meshToPcMap(const Model& input, Model& output, const Image& tex_map, bool logProgress);

	// triangle dubdivision based
	static void meshToPcDiv(const Model& input, Model& output, const Image& tex_map, 
		float areaThreshold, bool bilinear, bool logProgress);

};

#endif