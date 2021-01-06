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

const char* Quantize::brief = "Quantize model (mesh or point cloud) positions";

int Quantize::main(std::string app, std::string cmd, int argc, char* argv[])
{
	// the command options
	std::string inputModelFilename;
	std::string outputModelFilename;
	// Quantization parameters
	uint32_t qp = 16; // geometry

	// command line parameters
	try
	{
		cxxopts::Options options(app + " " + cmd, brief);
		options.add_options()
			("i,inputModel", "path to input model (obj or ply file)",
				cxxopts::value<std::string>())
			("o,outputModel", "path to output model (obj or ply file)",
				cxxopts::value<std::string>())
			("float", "if set the processings and outputs will be float32, int32 otherwise",
				cxxopts::value<bool>()->default_value("true"))
			("h,help", "Print usage")
			("qp", "Geometry quantization bitdepth",
				cxxopts::value<uint32_t>()->default_value("16"))
				;
		
		auto result = options.parse(argc, argv);

		// Analyse the options
		if (result.count("help") || result.arguments().size() == 0)
		{
			std::cout << options.help() << std::endl;
			return 0;
		}
		//	
		if (result.count("inputModel"))
			inputModelFilename = result["inputModel"].as<std::string>();
		else {
			std::cerr << "Error: missing inputModel parameter" << std::endl;
			std::cout << options.help() << std::endl;
			return 1;
		}
		//
		if (result.count("outputModel"))
			outputModelFilename = result["outputModel"].as<std::string>();
		else {
			std::cerr << "Error: missing outputModel parameter" << std::endl;
			std::cout << options.help() << std::endl;
			return 1;
		}
		//
		if (result.count("qp"))
			qp = result["qp"].as<uint32_t>();
	}
	catch (const cxxopts::OptionException& e)
	{
		std::cout << "error parsing options: " << e.what() << std::endl;
		return 1;
	}

	// the input
	Model inputModel;
	if (!IO::loadModel(inputModelFilename, inputModel)){
		return 1;
	}
	if (inputModel.vertices.size() == 0 ) {
		std::cout << "Error: invalid input model from " << inputModelFilename << std::endl;
		return 1;
	}

	// the output
	Model outputModel;

	// Perform the processings
	clock_t t1 = clock();

	std::cout << "Quantizing" << std::endl;
	std::cout << "  qp = " << qp << std::endl;
	Quantize::quantizePosition(inputModel, outputModel, qp);

	clock_t t2 = clock();
	std::cout << "Time on processing: " << ((float)(t2 - t1)) / CLOCKS_PER_SEC << " sec." << std::endl;

	// save the result
	if (IO::saveModel(outputModelFilename, outputModel))
		return 0;
	else
		return 1;

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
