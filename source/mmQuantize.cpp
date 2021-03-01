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
#include "mmQuantize.h"
#include "mmGeometry.h"

const char* Quantize::name = "quantize";
const char* Quantize::brief = "Quantize model (mesh or point cloud)";

//
Command* Quantize::create() { return new Quantize(); }

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
			("dequantize", "set to process dequantification at the ouput")
			("qp", "Geometry quantization bitdepth",
				cxxopts::value<uint32_t>()->default_value("12"))
			("qt", "UV coordinates quantization bitdepth",
				cxxopts::value<uint32_t>()->default_value("12"))
			("qn", "Normals quantization bitdepth",
				cxxopts::value<uint32_t>()->default_value("12"))
			("qc", "Colors quantization bitdepth",
				cxxopts::value<uint32_t>()->default_value("8"))
			("minPos", "min corner of vertex position bbox, a string of three floats. Computed of not set.",
				cxxopts::value<std::string>())
			("maxPos", "max corner of vertex position bbox, a string of three floats. Computed of not set.",
				cxxopts::value<std::string>())
			("minUv",  "min corner of vertex texture coordinates bbox, a string of three floats. Computed of not set.",
				cxxopts::value<std::string>())
			("maxUv",  "max corner of vertex texture coordinates bbox, a string of three floats. Computed of not set.",
				cxxopts::value<std::string>())
			("minNrm", "min corner of vertex normal bbox, a string of three floats. Computed of not set.",
				cxxopts::value<std::string>())
			("maxNrm", "max corner of vertex normal bbox, a string of three floats. Computed of not set.",
				cxxopts::value<std::string>())
			("minCol", "min corner of vertex color bbox, a string of three floats. Computed of not set.",
				cxxopts::value<std::string>())
			("maxCol", "max corner of vertex color bbox, a string of three floats. Computed of not set.",
				cxxopts::value<std::string>())
			("outputVar", "path to the output variables file.",
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

		if (result.count("outputVar"))
			outputVarFilename = result["outputVar"].as<std::string>();

		if (result.count("dequantize"))
			dequantize = result["dequantize"].as<bool>();

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
	std::cout << "  qt = " << qt << std::endl;
	std::cout << "  qn = " << qn << std::endl;
	std::cout << "  qc = " << qn << std::endl;
	
	if (dequantize) {
		Model* quantizedModel = new Model();
		
		quantize(*inputModel, *quantizedModel, qp, qt, qn, qc, 
			minPos, maxPos, minUv, maxUv, minNrm, maxNrm, minCol, maxCol,
			outputVarFilename );

		Dequantize::dequantize(*quantizedModel, *outputModel, qp, qt, qn, qc, 
			minPos, maxPos, minUv, maxUv, minNrm, maxNrm, minCol, maxCol);
		
		delete quantizedModel;
	}
	else {
		quantize(*inputModel, *outputModel, qp, qt, qn, qc, 
			minPos, maxPos, minUv, maxUv, minNrm, maxNrm, minCol, maxCol,
			outputVarFilename );
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

//
void Quantize::quantize(
	const Model& input, Model& output, 
	const uint32_t qp, const uint32_t qt, const uint32_t qn, const uint32_t qc,
	const glm::vec3& minPos, const glm::vec3& maxPos, const glm::vec2& minUv, const glm::vec2& maxUv,
	const glm::vec3& minNrm, const glm::vec3& maxNrm, const glm::vec3& minCol, const glm::vec3& maxCol,
	const std::string& outputVarFilename )
{
	// copy the input	
	output = input;
	
	// prepare loging and output var
	std::vector<std::ostream*> out;
	out.push_back(&std::cout);
	std::ofstream fout;
	if (outputVarFilename != "") {
		fout.open(outputVarFilename.c_str(), std::ios::out);
		if (fout) {
			out.push_back(&fout);
		}
		else {
			std::cout << "Error: could not create output file " << outputVarFilename << std::endl;
		}
	}

	//quantize position
	if (!input.vertices.empty() && qp >= 7) {
		glm::vec3 minBox, maxBox;
		if (minPos == maxPos) {
			std::cout << "Computing positions range" << std::endl;
			computeBBox(input.vertices, minBox, maxBox);
		}
		else {
			std::cout << "Using parameter positions range" << std::endl;
			minBox = minPos; maxBox = maxPos;
		}
		const glm::vec3 diag = maxBox - minBox;
		const float range = std::max(std::max(diag.x, diag.y), diag.z);
		const int32_t maxPositionQuantizedValue = (1u << static_cast<uint32_t>(qp)) - 1;

		for (size_t i = 0; i < out.size(); ++i) {
			*out[i] << "  minPos=\"" << minBox.x << " " << minBox.y << " " << minBox.z << "\"" << std::endl;
			*out[i] << "  maxPos=\"" << maxBox.x << " " << maxBox.y << " " << maxBox.z << "\"" << std::endl;
			*out[i] << "  rangePos=" << range << std::endl;
		}

		for (size_t i = 0; i < input.vertices.size() / 3; i++) {
			for (glm::vec3::length_type c = 0; c < 3; ++c) {
				uint32_t pos = static_cast<uint32_t> (std::floor(((input.vertices[i * 3 + c] - minBox[c]) / range) * maxPositionQuantizedValue + 0.5f));
				output.vertices[i * 3 + c] = static_cast<float> (pos);
			}
		}
	}

	//quantize UV coordinates 
	if (!input.uvcoords.empty() && qt >= 7) {
		glm::vec2 minBox, maxBox;
		if (minUv == maxUv) {
			std::cout << "Computing uv coordinates range" << std::endl;
			computeBBox(input.uvcoords, minBox, maxBox);
		}
		else {
			std::cout << "Using parameter uv coordinates range" << std::endl;
			minBox = minUv; maxBox = maxUv;
		}
		const glm::vec2 diag = maxBox - minBox;
		const float range = std::max(diag.x, diag.y);
		const int32_t maxUVcordQuantizedValue = (1u << static_cast<uint32_t>(qt)) - 1;
		
		for (size_t i = 0; i < out.size(); ++i) {
			*out[i] << "  minUv=\"" << minBox.x << " " << minBox.y << "\"" << std::endl;
			*out[i] << "  maxUv=\"" << maxBox.x << " " << maxBox.y << "\"" << std::endl;
			*out[i] << "  rangeUv=" << range << std::endl;
		}

		for (size_t i = 0; i < input.uvcoords.size() / 2; i++) {
			for (glm::vec2::length_type c = 0; c < 2; ++c) {
				uint32_t uv = static_cast<uint32_t> (std::floor(((input.uvcoords[i * 2 + c] - minBox[c]) / range) * maxUVcordQuantizedValue + 0.5f));
				output.uvcoords[i * 2 + c] = static_cast<float> (uv);
			}
		}
	}

	//quantize normals
	if (!input.normals.empty() && qn >= 7) {
		glm::vec3 minBox, maxBox;
		if (minNrm == maxNrm) {
			std::cout << "Computing normals range" << std::endl;
			computeBBox(input.normals, minBox, maxBox);
		}
		else {
			std::cout << "Using parameter normals range" << std::endl;
			minBox = minNrm; maxBox = maxNrm;
		}
		const glm::vec3 diag = maxBox - minBox;
		const float range = std::max(std::max(diag.x, diag.y), diag.z);
		const int32_t maxNormalQuantizedValue = (1u << static_cast<uint32_t>(qn)) - 1;
		
		for (size_t i = 0; i < out.size(); ++i) {
			*out[i] << "  minNrm=\"" << minBox.x << " " << minBox.y << " " << minBox.z << "\"" << std::endl;
			*out[i] << "  maxNrm=\"" << maxBox.x << " " << maxBox.y << " " << maxBox.z << "\"" << std::endl;
			*out[i] << "  rangeNrm=" << range << std::endl;
		}

		for (size_t i = 0; i < input.normals.size() / 3; i++) {
			for (glm::vec3::length_type c = 0; c < 3; ++c) {
				uint32_t nrm = static_cast<uint32_t> (std::floor(((input.normals[i * 3 + c] - minBox[c]) / range) * maxNormalQuantizedValue + 0.5f));
				output.normals[i * 3 + c] = static_cast<float> (nrm);
			}
		}
	}

	//quantize colors
	if (!input.colors.empty() && qc >= 7) {
		glm::vec3 minBox, maxBox;
		if (minCol == maxCol) {
			std::cout << "Computing colors range" << std::endl;
			computeBBox(input.normals, minBox, maxBox);
		}
		else {
			std::cout << "Using parameter colors range" << std::endl;
			minBox = minCol; maxBox = maxCol;
		}
		const glm::vec3 diag = maxBox - minBox;
		const float range = std::max(std::max(diag.x, diag.y), diag.z);
		const int32_t maxColorQuantizedValue = (1u << static_cast<uint32_t>(qn)) - 1;
		
		for (size_t i = 0; i < out.size(); ++i) {
			*out[i] << "  minCol=\"" << minBox.x << " " << minBox.y << " " << minBox.z << "\"" << std::endl;
			*out[i] << "  maxCol=\"" << maxBox.x << " " << maxBox.y << " " << maxBox.z << "\"" << std::endl;
			*out[i] << "  rangeCol=" << range << std::endl;
		}

		for (size_t i = 0; i < input.colors.size() / 3; i++) {
			for (glm::vec3::length_type c = 0; c < 3; ++c) {
				uint32_t col = static_cast<uint32_t> (std::floor(((input.colors[i * 3 + c] - minBox[c]) / range) * maxColorQuantizedValue + 0.5f));
				output.colors[i * 3 + c] = static_cast<float> (col);
			}
		}
	}
}
