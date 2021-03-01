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
// Author: meshToPcFace method based on original face sampling code from Owlii
// *****************************************************************

#include <iostream>
#include <fstream>
#include <sstream> 
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
#include "mmGeometry.h"
#include "mmIO.h"
#include "mmDegrade.h"
#include "mmModel.h"
#include "mmImage.h"

// Descriptions of the command
const char* Degrade::name = "degrade";
const char* Degrade::brief = "Degrade a mesh (todo points)";

// 
Command* Degrade::create() { return new Degrade(); }

// 
bool Degrade::initialize(Context* ctx, std::string app, int argc, char* argv[])
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
			("mode", "the sampling mode in [delface]",
				cxxopts::value<std::string>())
			("nthFace", "in delface mode, remove one face every nthFace.",
				cxxopts::value<size_t>()->default_value("50"))
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
		if (result.count("mode"))
			mode = result["mode"].as<std::string>();

		if (result.count("nthFace"))
			nthFace = result["nthFace"].as<size_t>();

	}
	catch (const cxxopts::OptionException& e)
	{
		std::cout << "error parsing options: " << e.what() << std::endl;
		return false;
	}

	return true;
}

bool Degrade::process(uint32_t frame) {

	// the input
	Model* inputModel = NULL;
	if ((inputModel = IO::loadModel(inputModelFilename)) == NULL) {
		return false;
	}
	if (inputModel->vertices.size() == 0 ||
		inputModel->triangles.size() == 0) {
		std::cout << "Error: invalid input model from " << inputModelFilename << std::endl;
		return false;
	}

	// the output
	Model* outputModel = new Model();
	
	// Perform the processings
	clock_t t1 = clock();
	if (mode == "delface")
	{
		delface(*inputModel, nthFace, *outputModel);
	}
	else {
		std::cout << "Error: invalid mode " << mode << std::endl;
		return false;
	}

	clock_t t2 = clock();
	std::cout << "Time on processing: " << ((float)(t2 - t1)) / CLOCKS_PER_SEC << " sec." << std::endl;

	// save the result
	if (IO::saveModel(outputModelFilename, outputModel))
		return true;
	else
		return false;
}

bool Degrade::delface(const Model& input, size_t nthFace, Model& output) {
	
	bool hasNormals = input.normals.size() != 0;
	bool hasUvCoord = input.uvcoords.size() != 0;
	bool hasColors = input.colors.size() != 0;

	ModelBuilder builder(output);

	// implement the proper code to remove random faces
	for (size_t triIdx = 0; triIdx < input.triangles.size() / 3; ++triIdx) {
		if (triIdx % nthFace != 0) {
			Vertex v1, v2, v3;
			fetchTriangle(input, triIdx, hasUvCoord, hasColors, hasNormals, v1, v2, v3);
			builder.pushTriangle(v1, v2, v3);
		}
	}

	return true;
}
