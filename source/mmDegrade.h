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

#ifndef _MM_DEGRADE_H_
#define _MM_DEGRADE_H_

// internal headers
#include "mmCommand.h"
#include "mmModel.h"
#include "mmImage.h"

class Degrade : Command {

private:

	// the command options
	std::string inputModelFilename;
	std::string outputModelFilename;
	std::string mode="delface";
	size_t nthFace=50;

public:

	Degrade() {};

	// Description of the command
	static const char* name;
	static const char* brief;
	// command creator
	static Command* create();

	// the command main program
	virtual bool initialize(Context* ctx, std::string app, int argc, char* argv[]);
	virtual bool process(uint32_t frame);
	virtual bool finalize() { return true; };

private:

	bool delface(const Model& input, size_t nthFace, Model& output);

};

#endif