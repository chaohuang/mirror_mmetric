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

#ifndef _MM_COMPARE_H_
#define _MM_COMPARE_H_

// internal headers
#include "mmCommand.h"
#include "mmModel.h"
// MPEG PCC metric
#include "pcc/pcc_distortion.hpp"

class Compare : Command {
	
public:

	// Descriptions of the command
	virtual const char* name(void) {
		return "compare";
	};
	virtual const char* brief(void) {
		return "Compare model A vs model B"; 
	};

	// the command main program
	virtual int main(std::string app, int argc, char* argv[]);

	// compare two meshes for equality (using mem comp if epsilon = 0)
	// if epsilon = 0, return 0 on success and 1 on difference
	// if epsilon > 0, return 0 on success and nb diff on difference if sizes are equal, 1 otherwise
	static int equ(
		const Model& modelA, const Model& modelB,
		const Image& mapA, const Image& mapB,
		float epsilon,
		Model& outputA, Model& outputB);

	// compare two meshes using MPEG pcc_distortion metric
	static int pcc(
		const Model& modelA, const Model& modelB,
		const Image& mapA, const Image& mapB,
		pcc_quality::commandPar& params,
		Model& outputA, Model& outputB
	);

	// compare two meshes using PCQM metric
	static int pcqm(
		const Model& modelA, const Model& modelB,
		const Image& mapA, const Image& mapB,
		const double radiusCurvature,
		const int thresholdKnnSearch,
		const double radiusFactor,
		Model& outputA, Model& outputB
	);

};

#endif
