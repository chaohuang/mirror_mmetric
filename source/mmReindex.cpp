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
#include <math.h>
// mathematics
#include <glm/vec3.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
// argument parsing
#include <cxxopts.hpp>

// internal headers
#include "mmIO.h"
#include "mmModel.h"
#include "mmImage.h"
#include "mmReindex.h"
#include "mmGeometry.h"

const char* Reindex::name="reindex";
const char* Reindex::brief="Reindex mesh and optionaly sort vertices and face indices";

// register the command
Command* Reindex::create() { return new Reindex(); }
static bool init = Command::addCreator(Reindex::name, Reindex::brief, Reindex::create);

//
bool Reindex::initialize(Context* ctx, std::string app, int argc, char* argv[])
{
	// command line parameters
	try
	{
		cxxopts::Options options(app + " " + name, brief);
		options.add_options()
			("i,inputModel", "path to input model (obj or ply file)",
				cxxopts::value<std::string>())
			("o,outputModel", "path to output model (obj or ply file)",
				cxxopts::value<std::string>())
			("h,help", "Print usage")
			("sort", "Sort method in none, vertices, oriented, unoriented.",
				cxxopts::value<std::string>()->default_value("none"))
				;
		
		auto result = options.parse(argc, argv);

		// Analyse the options
		if (result.count("help") || result.arguments().size() == 0)
		{
			std::cout << options.help() << std::endl;
			return false;
		}
		//	
		if (result.count("inputModel"))
			inputModelFilename = result["inputModel"].as<std::string>();
		else {
			std::cerr << "Error: missing inputModel parameter" << std::endl;
			std::cout << options.help() << std::endl;
			return false;
		}
		//
		if (result.count("outputModel"))
			outputModelFilename = result["outputModel"].as<std::string>();
		else {
			std::cerr << "Error: missing outputModel parameter" << std::endl;
			std::cout << options.help() << std::endl;
			return false;
		}
		//
		if (result.count("sort"))
			sort = result["sort"].as<std::string>();
	}
	catch (const cxxopts::OptionException& e)
	{
		std::cout << "error parsing options: " << e.what() << std::endl;
		return false;
	}

	return true;
}

bool Reindex::process(uint32_t frame) {

	// the input
	Model* inputModel;
	if ((inputModel = IO::loadModel(inputModelFilename)) == NULL) {
		return false;
	}
	if (inputModel->vertices.size() == 0) {
		std::cout << "Error: invalid input model from " << inputModelFilename << std::endl;
		return false;
	}

	// the output
	Model* outputModel = new Model();
	outputModel->header = inputModel->header; // preserve material
	outputModel->comments = inputModel->comments;

	// Perform the processings
	clock_t t1 = clock();

	std::cout << "Reindex" << std::endl;
	std::cout << "  sort = " << sort << std::endl;
	if ( sort == "none"){
		reindex(*inputModel, *outputModel);
	} 
	else if (sort == "vertex" || sort == "oriented" || sort == "unoriented") {
		reorder(*inputModel, sort, *outputModel);
	}
	else {
		std::cout << "Error: invalid sorting method " << sort << std::endl;
	}

	clock_t t2 = clock();
	std::cout << "Time on processing: " << ((float)(t2 - t1)) / CLOCKS_PER_SEC << " sec." << std::endl;

	// save the result
	if (IO::saveModel(outputModelFilename, outputModel))
		return true;
	else {
		delete outputModel;
		return false;
	}

	return true;
}

