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

#include <iostream>
#include <algorithm>
#include <fstream>
#include <unordered_map>
#include <time.h>

// argument parsing
#include <cxxopts.hpp>

// internal headers
#include "mmContext.h"
#include "mmSequence.h"

const char* Sequence::name = "sequence";
const char* Sequence::brief = "Sequence global parameters";

// register the command
Command* Sequence::create() { return new Sequence(); }
static bool init = Command::addCreator(Sequence::name, Sequence::brief, Sequence::create);

//
bool Sequence::initialize(Context* ctx, std::string app, int argc, char* argv[])
{
	// command line parameters
	try
	{
		cxxopts::Options options(app + " " + name, brief);
		options.add_options()
			("firstFrame", "Sets the first frame of the sequence, included.",
				cxxopts::value<int>()->default_value("0"))
			("lastFrame", "Sets the last frame of the sequence, included. Must be >= to firstFrame.",
				cxxopts::value<int>()->default_value("0"))
			("h,help", "Print usage")
			;
		
		auto result = options.parse(argc, argv);

		// Analyse the options
		if (result.count("help") || result.arguments().size() == 0)
		{
			std::cout << options.help() << std::endl;
			return false;
		}
		//	
		int firstFrame = 0;
		int lastFrame = 0;
		if (result.count("firstFrame"))
			firstFrame = result["firstFrame"].as<int>();
		if (result.count("lastFrame"))
			lastFrame = result["lastFrame"].as<int>();
		if (lastFrame < firstFrame || lastFrame < 0 || firstFrame < 0 ) {
			std::cerr << "Error: must have 0 >= lastFrame >= firstFrame. Got firstFrame=" << firstFrame << " and lastFrame = " << lastFrame << std::endl;
			return false;
		}
		ctx->setFrameRange(firstFrame, lastFrame);
	}
	catch (const cxxopts::OptionException& e)
	{
		std::cout << "Error: parsing options, " << e.what() << std::endl;
		return false;
	}

	return true;
}
