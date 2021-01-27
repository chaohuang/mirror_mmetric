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
#include "mmCommand.h"

// software version
#define MM_VERSION "0.1.2"

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

		int startIdx = 1;
		int endIdx;

		do {
			// search for end of command (END or end of argv)
			endIdx = startIdx;
			std::string arg = argv[endIdx];
			while (endIdx != argc - 1 && arg != "END") {
				endIdx++;
				arg = argv[endIdx];
			}
			int subArgc = endIdx - startIdx + ((endIdx == argc - 1) ? 1 : 0);

			// execute the command
			int res = Command::execute(APP_NAME, std::string(argv[startIdx]), subArgc, &argv[startIdx]);
			if (res != 0) 
				return res;
						
			// start of next command
			startIdx = endIdx + 1;

		} while (startIdx < argc);

		// all commands were executed
		return 0;
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
	Command::logCommands();
	std::cout << std::endl;

}
