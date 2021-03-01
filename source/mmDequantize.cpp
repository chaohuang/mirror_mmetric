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
#include "mmDequantize.h"
#include "mmGeometry.h"

const char* Dequantize::name = "dequantize";
const char* Dequantize::brief = "Dequantize model (mesh or point cloud) ";

// register the command
Command* Dequantize::create() { return new Dequantize(); }

//
bool Dequantize::initialize(Context* ctx, std::string app, int argc, char* argv[])
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
			("qp", "Geometry quantization bitdepth. No dequantization of geometry if not set or < 7.",
				cxxopts::value<uint32_t>())
			("qt", "UV coordinates quantization bitdepth. No dequantization of uv coordinates if not set or < 7.",
				cxxopts::value<uint32_t>())
			("qn", "Normals quantization bitdepth. No dequantization of normals if not set or < 7.",
				cxxopts::value<uint32_t>())
			("qc", "Colors quantization bitdepth. No dequantization of colors if not set or < 7.",
				cxxopts::value<uint32_t>())
			("minPos", "min corner of vertex position bbox, a string of three floats. Mandatory if qp set and >= 7",
				cxxopts::value<std::string>())
			("maxPos", "max corner of vertex position bbox, a string of three floats. Mandatory if qp set and >= 7",
				cxxopts::value<std::string>())
			("minUv",  "min corner of vertex texture coordinates bbox, a string of three floats. Mandatory if qt set and >= 7",
				cxxopts::value<std::string>())
			("maxUv",  "max corner of vertex texture coordinates bbox, a string of three floats. Mandatory if qt set and >= 7",
				cxxopts::value<std::string>())
			("minNrm", "min corner of vertex normal bbox, a string of three floats. Mandatory if qn set and >= 7.",
				cxxopts::value<std::string>())
			("maxNrm", "max corner of vertex normal bbox, a string of three floats. Mandatory if qn set and >= 7",
				cxxopts::value<std::string>())
			("minCol", "min corner of vertex colors bbox, a string of three floats. Mandatory if qc set and >= 7",
				cxxopts::value<std::string>())
			("maxCol", "max corner of vertex colors bbox, a string of three floats. Mandatory if qc set and >= 7",
				cxxopts::value<std::string>())
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
		if (result.count("qt"))
			qt = result["qt"].as<uint32_t>();
		if (result.count("qn"))
			qn = result["qn"].as<uint32_t>();
		if (result.count("qc"))
			qc = result["qc"].as<uint32_t>();

		if (result.count("minPos")) {
			minPosStr = result["minPos"].as<std::string>();
			if (!parseVec3(minPosStr, minPos)) {
				std::cout << "Error: parsing --minPos=\"" << minPosStr 
					<< "\" expected three floats with space separator" << std::endl;
				return false;
			}
		}
		if (result.count("maxPos")) {
			maxPosStr = result["maxPos"].as<std::string>();
			if (!parseVec3(maxPosStr, maxPos)) {
				std::cout << "Error: parsing --maxPos=\"" << maxPosStr 
					<< "\" expected three floats with space separator" << std::endl;
				return false;
			}
		}
		if (qp >= 7) {
			if (minPosStr == "" || maxPosStr == "") {
				std::cout << "Error: qp >= 7 but minPos and/or maxPos not set." << std::endl;
				return false;
			}
		}

		if (result.count("minUv")) {
			minUvStr = result["minUv"].as<std::string>();
			if (!parseVec2(minUvStr, minUv)) {
				std::cout << "Error: parsing --minUv=\"" << minUvStr 
					<< "\" expected three floats with space separator" << std::endl;
				return false;
			}
		}
		if (result.count("maxUv")) {
			maxUvStr = result["maxUv"].as<std::string>();
			if (!parseVec2(maxUvStr, maxUv)) {
				std::cout << "Error: parsing --maxUv=\"" << maxUvStr 
					<< "\" expected three floats with space separator" << std::endl;
				return false;
			}
		}
		if (qt >= 7) {
			if (minUvStr == "" || maxUvStr == "") {
				std::cout << "Error: qt >= 7 but minUv and/or maxUv not set." << std::endl;
				return false;
			}
		}

		if (result.count("minNrm")) {
			minNrmStr = result["minNrm"].as<std::string>();
			if (!parseVec3(minNrmStr, minNrm)) {
				std::cout << "Error: parsing --minNrm=\"" << minNrmStr 
					<< "\" expected three floats with space separator" << std::endl;
				return false;
			}
		}
		if (result.count("maxNrm")) {
			maxNrmStr = result["maxNrm"].as<std::string>();
			if (!parseVec3(maxNrmStr, maxNrm)) {
				std::cout << "Error: parsing --maxNrm=\"" << maxNrmStr 
					<< "\" expected three floats with space separator" << std::endl;
				return false;
			}
		}
		if (qn >= 7) {
			if (minNrmStr == "" || maxNrmStr == "") {
				std::cout << "Error: qn >= 7 but minNrm and/or maxNrm not set." << std::endl;
				return false;
			}
		}

		if (result.count("minCol")) {
			minColStr = result["minCol"].as<std::string>();
			if (!parseVec3(minColStr, minCol)) {
				std::cout << "Error: parsing --minCol=\"" << minColStr 
					<< "\" expected three floats with space separator" << std::endl;
				return false;
			}
		}
		if (result.count("maxCol")) {
			maxColStr = result["maxCol"].as<std::string>();
			if (!parseVec3(maxColStr, maxCol)) {
				std::cout << "Error: parsing --maxCol=\"" << maxColStr 
					<< "\" expected three floats with space separator" << std::endl;
				return false;
			}
		}
		if (qc >= 7) {
			if (minColStr == "" || maxColStr == "") {
				std::cout << "Error: qc >= 7 but minCol and/or maxCol not set." << std::endl;
				return false;
			}
		}
	}
	catch (const cxxopts::OptionException& e)
	{
		std::cout << "error parsing options: " << e.what() << std::endl;
		return false;
	}

	return true;
}

bool Dequantize::process(uint32_t frame) {

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

	std::cout << "De-quantizing" << std::endl;
	std::cout << "  qp = " << qp << std::endl;
	std::cout << "  qt = " << qt << std::endl;
	std::cout << "  qn = " << qn << std::endl;
	std::cout << "  qc = " << qn << std::endl;

	Dequantize::dequantize(*inputModel, *outputModel, qp, qt, qn, qc, 
		minPos, maxPos, minUv, maxUv, minNrm, maxNrm, minCol, maxCol );

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

//
void Dequantize::dequantize(
	const Model& input, Model& output, 
	const uint32_t qp, const uint32_t qt, const uint32_t qn, const uint32_t qc,
	const glm::vec3& minPos, const glm::vec3& maxPos, const glm::vec2& minUv, const glm::vec2& maxUv,
	const glm::vec3& minNrm, const glm::vec3& maxNrm, const glm::vec3& minCol, const glm::vec3& maxCol )
{

	// copy input
	output = input;

	// dequantize position
	if (!input.vertices.empty() && qp >= 7 ) {
		const glm::vec3 minBox = minPos;
		const glm::vec3 maxBox = maxPos;
		glm::vec3 diag = maxBox - minBox;
		const float range = std::max(std::max(diag.x, diag.y), diag.z);
		const int32_t maxQuantizedValue = (1u << static_cast<uint32_t>(qp)) - 1;
		
		std::cout << "  minPos=\"" << minBox.x << " " << minBox.y << " " << minBox.z << "\"" << std::endl;
		std::cout << "  maxPos=\"" << maxBox.x << " " << maxBox.y << " " << maxBox.z << "\"" << std::endl;
		std::cout << "  rangePos=" << range << std::endl;

		for (size_t i = 0; i < input.vertices.size() / 3; i++) {
			for (glm::vec3::length_type c = 0; c < 3; ++c) {
				output.vertices[i * 3 + c] = (input.vertices[i * 3 + c] * range / maxQuantizedValue) + minBox[c];
			}
		}
	}

	// dequantize UV coordinates 
	if (!input.uvcoords.empty() && qt >= 7) {
		const glm::vec2 minBox = minUv;
		const glm::vec2 maxBox = maxUv;
		const glm::vec2 diag = maxBox - minBox;
		const float range = std::max(diag.x, diag.y);
		const int32_t maxQuantizedValue = (1u << static_cast<uint32_t>(qt)) - 1;
		
		std::cout << "  minUv=\"" << minBox.x << " " << minBox.y << "\"" << std::endl;
		std::cout << "  maxUv=\"" << maxBox.x << " " << maxBox.y << "\"" << std::endl;
		std::cout << "  rangeUv=" << range << std::endl;

		for (size_t i = 0; i < input.uvcoords.size() / 2; i++) {
			for (glm::vec2::length_type c = 0; c < 2; ++c) {
				output.uvcoords[i * 2 + c] = (input.uvcoords[i * 2 + c] * range / maxQuantizedValue) + minBox[c];
			}
		}
	}

	// dequantize normals
	if (!input.normals.empty() && qn >= 7) {
		const glm::vec3 minBox = minNrm;
		const glm::vec3 maxBox = maxNrm;
		const glm::vec3 diag = maxBox - minBox;
		const float range = std::max(std::max(diag.x, diag.y), diag.z);
		const int32_t maxQuantizedValue = (1u << static_cast<uint32_t>(qn)) - 1;

		std::cout << "  minNrm=\"" << minBox.x << " " << minBox.y << " " << minBox.z << "\"" << std::endl;
		std::cout << "  maxNrm=\"" << maxBox.x << " " << maxBox.y << " " << maxBox.z << "\"" << std::endl;
		std::cout << "  rangeNrm=" << range << std::endl;

		for (size_t i = 0; i < input.normals.size() / 3; i++) {
			for (glm::vec3::length_type c = 0; c < 3; ++c) {
				output.normals[i * 3 + c] = (input.normals[i * 3 + c] * range / maxQuantizedValue) + minBox[c];
			}
		}
	}

	// dequantize colors
	if (!input.colors.empty() && qc >= 7) {
		const glm::vec3 minBox = minCol;
		const glm::vec3 maxBox = maxCol;
		const glm::vec3 diag = maxBox - minBox;
		const float range = std::max(std::max(diag.x, diag.y), diag.z);
		const int32_t maxQuantizedValue = (1u << static_cast<uint32_t>(qn)) - 1;

		std::cout << "  minCol=\"" << minBox.x << " " << minBox.y << " " << minBox.z << "\"" << std::endl;
		std::cout << "  maxCol=\"" << maxBox.x << " " << maxBox.y << " " << maxBox.z << "\"" << std::endl;
		std::cout << "  rangeCol=" << range << std::endl;

		for (size_t i = 0; i < input.colors.size() / 3; i++) {
			for (glm::vec3::length_type c = 0; c < 3; ++c) {
				output.colors[i * 3 + c] = (input.colors[i * 3 + c] * range / maxQuantizedValue) + minBox[c];
			}
		}
	}
}
