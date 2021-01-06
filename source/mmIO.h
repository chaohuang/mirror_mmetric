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

#ifndef _MM_IO_H_
#define _MM_IO_H_

#include <string>

#include "mmModel.h"
#include "mmImage.h"

struct IO {

	IO() {}

	// Automatic choice on extension
	static bool loadModel(std::string filename, Model& output);
	static bool saveModel(std::string filename, const Model& input);

	// OBJ
	static bool loadObj(std::string filename, Model& output);
	static bool saveObj(std::string filename, const Model& input);

	// PLY
	static bool loadPly(std::string filename, Model& output);
	static bool savePly(std::string filename, const Model& input);

	// Images
	static bool loadImage(std::string filename, Image& output);

};

#endif