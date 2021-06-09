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

//
#include <iostream>
#include <vector>
#include <limits>
#include <time.h>

// internal headers
#include "mmIO.h"
#include "mmContext.h"
#include "mmCommand.h"

// software version
#define MM_VERSION "0.1.10"

// the name of the application binary
// i.e argv[0] minus the eventual path
#ifdef _WIN32
#define APP_NAME "mm.exe"
#else
#define APP_NAME "mm"
#endif

// The commands
#include "mmAnalyse.h"
#include "mmCompare.h"
#include "mmDegrade.h"
#include "mmQuantize.h"
#include "mmDequantize.h"
#include "mmReindex.h"
#include "mmSample.h"
#include "mmSequence.h"
#include "mmNormals.h"
// The non MPEG commands
#include "mmRender.h"

// analyse command line and run processings
int main(int argc, char* argv[])
{
	// this is mandatory to print floats with full precision 
	std::cout.precision(std::numeric_limits< float >::max_digits10);

	// register the commands
	Command::addCreator(Analyse::name, Analyse::brief, Analyse::create);
	Command::addCreator(Compare::name, Compare::brief, Compare::create);
	Command::addCreator(Degrade::name, Degrade::brief, Degrade::create);
	Command::addCreator(Quantize::name, Quantize::brief, Quantize::create);
	Command::addCreator(Dequantize::name, Dequantize::brief, Dequantize::create);
	Command::addCreator(Reindex::name, Reindex::brief, Reindex::create);
	Command::addCreator(Sample::name, Sample::brief, Sample::create);
	Command::addCreator(Sequence::name, Sequence::brief, Sequence::create);
	Command::addCreator(Normals::name, Normals::brief, Normals::create);
	// register non MPEG commands
	Command::addCreator(Render::name, Render::brief, Render::create);
	 
	// execute the commands
	if (argc > 1) {

		// global timer
		clock_t t1 = clock();
		
		// context shared among commands
		Context context;
		IO::setContext(&context);
		// set of commands to be executer in order
		std::vector<Command*> commands;

		// 1 - initialize the command list
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

			// create a new command
			Command* newCmd = NULL;
			if ((newCmd = Command::create(APP_NAME, std::string(argv[startIdx]))) == NULL) {
				return 1;
			}
			commands.push_back(newCmd);

			// initialize the command
			if (!newCmd->initialize(&context, APP_NAME, subArgc, &argv[startIdx])) {
				return 1;
			}

			// start of next command
			startIdx = endIdx + 1;

		} while (startIdx < argc);

		// 2 - execute each command for each frame
		int procErrors = 0;
		for (uint32_t frame = context.getFirstFrame(); frame <= context.getLastFrame(); ++frame) {
			std::cout << "Processing frame " << frame << std::endl;
			context.setFrame(frame);
			for (size_t cmdIndex = 0; cmdIndex < commands.size(); ++cmdIndex) {
				if (!commands[cmdIndex]->process(frame)){
					procErrors++;
				}
			}
			// purge the models, clean IO for next frame
			IO::purge();
		}
		if (procErrors != 0) {
			std::cerr << "There was " << procErrors << " processing errors" << std::endl;
		}

		// 3 - collect results
		int finErrors = 0;
		for (size_t cmdIndex = 0; cmdIndex < commands.size(); ++cmdIndex) {
			if (!commands[cmdIndex]->finalize()) {
				finErrors++;
			}
		}
		if (finErrors != 0) {
			std::cerr << "There was " << finErrors << " finalization errors" << std::endl;
		}
		
		clock_t t2 = clock();
		std::cout << "Time on overall processing: " << ((float)(t2 - t1)) / CLOCKS_PER_SEC << " sec." << std::endl;

		// all commands were executed
		return procErrors + finErrors;
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

	return 0;
}
