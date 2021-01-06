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

#ifndef _MM_QUANTIZE_H_
#define _MM_QUANTIZE_H_

// internal headers
#include "mmModel.h"

struct Quantize {

	// Descriptions of the commands
	static const char* brief; 

	// the command main program
	static int main(std::string app, std::string cmd, int argc, char* argv[]);
	
	// TODO document
	static void quantizePosition(const Model& input, Model& output, uint32_t bitdepth);

};

#endif
