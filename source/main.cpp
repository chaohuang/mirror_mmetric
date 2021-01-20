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

//
#include <iostream>

// internal headers
#include "mmSample.h"
#include "mmQuantize.h"
#include "mmCompare.h"

// software version
#define MM_VERSION "0.1.1"

// the name of the application binary
// i.e argv[0] minus the eventual path
#ifdef _WIN32
#define APP_NAME "mm.exe"
#else
#define APP_NAME "mm"
#endif

// analyse command line and run processings
int main(int argc, char* argv[])
{
	if (argc > 1) {
		std::string command(argv[1]);
		if (command == "sample")
			return Sample::main(APP_NAME, command, argc - 1, &argv[1]);
		else if (command == "compare")
			return Compare::main(APP_NAME, command, argc - 1, &argv[1]);
		else if (command == "quantize")
			return Quantize::main(APP_NAME, command, argc - 1, &argv[1]);
		//
		std::cerr << "Error: unknown command " << argv[1] << std::endl;
	}

	// print help
	std::cout << "3D model processing commands v" << MM_VERSION << std::endl;
	std::cout << "Usage:" << std::endl;
	std::cout << "  " << APP_NAME << " command [OPTION...]" << std::endl;
	std::cout << std::endl;
	std::cout << "Command help:" << std::endl;
	std::cout << "  " << APP_NAME << " command --help" << std::endl;
	std::cout << std::endl;
	std::cout << "Command:" << std::endl;
	std::cout << "  sample     " << Sample::brief << std::endl;
	std::cout << "  compare    " << Compare::brief << std::endl;
	std::cout << "  quantize   " << Quantize::brief << std::endl;
	std::cout << std::endl;

}
