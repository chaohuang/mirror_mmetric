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
#include "mmQuantize.h"
#include "mmGeometry.h"

const char* Quantize::name="quantize";
const char* Quantize::brief="Quantize model (mesh or point cloud) positions";

// register the command
Command* Quantize::create() { return new Quantize(); }
static bool init = Command::addCreator(Quantize::name, Quantize::brief, Quantize::create);

//
bool Quantize::initialize(Context* ctx, std::string app, int argc, char* argv[])
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
			("qp", "Geometry quantization bitdepth",
				cxxopts::value<uint32_t>()->default_value("16"))
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
		if (result.count("qp"))
			qp = result["qp"].as<uint32_t>();
	}
	catch (const cxxopts::OptionException& e)
	{
		std::cout << "error parsing options: " << e.what() << std::endl;
		return false;
	}

	return true;
}

bool Quantize::process(uint32_t frame) {

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

	// Perform the processings
	clock_t t1 = clock();

	std::cout << "Quantizing" << std::endl;
	std::cout << "  qp = " << qp << std::endl;
	Quantize::quantizePosition(*inputModel, *outputModel, qp);

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

// todo 1: cleanup degenerated faces and duplicates vertices after quantization
// todo 2: add quantization parameteres and action for UVs (with correction), colors, etc...
void Quantize::quantizePosition(const Model& input, Model& output, uint32_t bitdepth) {

	// ugly brute force since vertices will be replicated, but easily captures all input fields
	output = input;

	// let's go
	glm::vec3 minBox, maxBox;
	computeBBox(input.vertices, minBox, maxBox);
	
	glm::vec3 diag = maxBox - minBox;
	float range = std::max(std::max(diag.x, diag.y), diag.z);

	for (size_t i = 0; i < input.vertices.size() / 3; i++) {
		for (glm::vec3::length_type c = 0; c < 3; ++c) {
			output.vertices[i * 3 + c] = (float)(uint32_t)(((input.vertices[i * 3 + c] - minBox[c]) / range) * (1 << bitdepth) + 0.5);
		}
	}
}
