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
			("minUv", "min corner of vertex texture coordinates bbox, a string of three floats. Mandatory if qt set and >= 7",
				cxxopts::value<std::string>())
			("maxUv", "max corner of vertex texture coordinates bbox, a string of three floats. Mandatory if qt set and >= 7",
				cxxopts::value<std::string>())
			("minNrm", "min corner of vertex normal bbox, a string of three floats. Mandatory if qn set and >= 7.",
				cxxopts::value<std::string>())
			("maxNrm", "max corner of vertex normal bbox, a string of three floats. Mandatory if qn set and >= 7",
				cxxopts::value<std::string>())
			("minCol", "min corner of vertex colors bbox, a string of three floats. Mandatory if qc set and >= 7",
				cxxopts::value<std::string>())
			("maxCol", "max corner of vertex colors bbox, a string of three floats. Mandatory if qc set and >= 7",
				cxxopts::value<std::string>())
			("useFixedPoint", "interprets minPos and maxPos inputs as fixed point 16.",
				cxxopts::value<bool>())
			("colorSpaceConversion", "Convert color space from YUV to RGB.",
				cxxopts::value<bool>())
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
			_inputModelFilename = result["inputModel"].as<std::string>();
		else {
			std::cerr << "Error: missing inputModel parameter" << std::endl;
			std::cout << options.help() << std::endl;
			return false;
		}
		//
		if (result.count("outputModel"))
			_outputModelFilename = result["outputModel"].as<std::string>();
		else {
			std::cerr << "Error: missing outputModel parameter" << std::endl;
			std::cout << options.help() << std::endl;
			return false;
		}
		//
		if (result.count("qp"))
			_qp = result["qp"].as<uint32_t>();
		if (result.count("qt"))
			_qt = result["qt"].as<uint32_t>();
		if (result.count("qn"))
			_qn = result["qn"].as<uint32_t>();
		if (result.count("qc"))
			_qc = result["qc"].as<uint32_t>();

		if (result.count("minPos")) {
			_minPosStr = result["minPos"].as<std::string>();
			if (!parseVec3(_minPosStr, _minPos)) {
				std::cout << "Error: parsing --minPos=\"" << _minPosStr
					<< "\" expected three floats with space separator" << std::endl;
				return false;
			}
		}
		if (result.count("maxPos")) {
			_maxPosStr = result["maxPos"].as<std::string>();
			if (!parseVec3(_maxPosStr, _maxPos)) {
				std::cout << "Error: parsing --maxPos=\"" << _maxPosStr
					<< "\" expected three floats with space separator" << std::endl;
				return false;
			}
		}
		if (_qp >= 7) {
			if (_minPosStr == "" || _maxPosStr == "") {
				std::cout << "Error: qp >= 7 but minPos and/or maxPos not set." << std::endl;
				return false;
			}
		}

		if (result.count("minUv")) {
			_minUvStr = result["minUv"].as<std::string>();
			if (!parseVec2(_minUvStr, _minUv)) {
				std::cout << "Error: parsing --minUv=\"" << _minUvStr
					<< "\" expected three floats with space separator" << std::endl;
				return false;
			}
		}
		if (result.count("maxUv")) {
			_maxUvStr = result["maxUv"].as<std::string>();
			if (!parseVec2(_maxUvStr, _maxUv)) {
				std::cout << "Error: parsing --maxUv=\"" << _maxUvStr
					<< "\" expected three floats with space separator" << std::endl;
				return false;
			}
		}
		if (_qt >= 7) {
			if (_minUvStr == "" || _maxUvStr == "") {
				std::cout << "Error: qt >= 7 but minUv and/or maxUv not set." << std::endl;
				return false;
			}
		}

		if (result.count("minNrm")) {
			_minNrmStr = result["minNrm"].as<std::string>();
			if (!parseVec3(_minNrmStr, _minNrm)) {
				std::cout << "Error: parsing --minNrm=\"" << _minNrmStr
					<< "\" expected three floats with space separator" << std::endl;
				return false;
			}
		}
		if (result.count("maxNrm")) {
			_maxNrmStr = result["maxNrm"].as<std::string>();
			if (!parseVec3(_maxNrmStr, _maxNrm)) {
				std::cout << "Error: parsing --maxNrm=\"" << _maxNrmStr
					<< "\" expected three floats with space separator" << std::endl;
				return false;
			}
		}
		if (_qn >= 7) {
			if (_minNrmStr == "" || _maxNrmStr == "") {
				std::cout << "Error: qn >= 7 but minNrm and/or maxNrm not set." << std::endl;
				return false;
			}
		}

		if (result.count("minCol")) {
			_minColStr = result["minCol"].as<std::string>();
			if (!parseVec3(_minColStr, _minCol)) {
				std::cout << "Error: parsing --minCol=\"" << _minColStr
					<< "\" expected three floats with space separator" << std::endl;
				return false;
			}
		}
		if (result.count("maxCol")) {
			_maxColStr = result["maxCol"].as<std::string>();
			if (!parseVec3(_maxColStr, _maxCol)) {
				std::cout << "Error: parsing --maxCol=\"" << _maxColStr
					<< "\" expected three floats with space separator" << std::endl;
				return false;
			}
		}
		if (result.count("colorSpaceConversion")) {
			_colorSpaceConversion = result["colorSpaceConversion"].as<bool>();
		}
		if ((_qc >= 7) && (!_colorSpaceConversion)) {
			if (_minColStr == "" || _maxColStr == "") {
				std::cout << "Error: qc >= 7 but minCol and/or maxCol not set." << std::endl;
				return false;
			}
		}
		if (result.count("useFixedPoint")) {
			_useFixedPoint = result["useFixedPoint"].as<bool>();
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
	if ((inputModel = IO::loadModel(_inputModelFilename)) == NULL) {
		return false;
	}
	if (inputModel->vertices.size() == 0) {
		std::cout << "Error: invalid input model from " << _inputModelFilename << std::endl;
		return false;
	}

	// the output
	Model* outputModel = new Model();

	// Perform the processings
	clock_t t1 = clock();

	std::cout << "De-quantizing" << std::endl;
	std::cout << "  qp = " << _qp << std::endl;
	std::cout << "  qt = " << _qt << std::endl;
	std::cout << "  qn = " << _qn << std::endl;
	std::cout << "  qc = " << _qc << std::endl;

	Dequantize::dequantize(*inputModel, *outputModel, _qp, _qt, _qn, _qc,
		_minPos, _maxPos, _minUv, _maxUv, _minNrm, _maxNrm, _minCol, _maxCol, _useFixedPoint,_colorSpaceConversion);

	clock_t t2 = clock();
	std::cout << "Time on processing: " << ((float)(t2 - t1)) / CLOCKS_PER_SEC << " sec." << std::endl;

	// save the result
	if (IO::saveModel(_outputModelFilename, outputModel))
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
	const glm::vec3& minNrm, const glm::vec3& maxNrm, const glm::vec3& minCol, const glm::vec3& maxCol,
	const bool useFixedPoint,const bool colorSpaceConversion)
{

	// copy input
	output = input;

	// dequantize position
	if (!input.vertices.empty() && qp >= 7) {
		glm::vec3 minBox = minPos;
		glm::vec3 maxBox = maxPos;
		const int32_t fixedPoint16 = (1u << 16);
		if (useFixedPoint) {
			// converting the values to a fixed point representation
			// minBox(FP16) will be used in AAPS -> shift
			for (int i = 0; i < 3; i++) {
				if (minBox[i] > 0)
					minBox[i] = (std::floor(minBox[i] * fixedPoint16)) / fixedPoint16;
				else
					minBox[i] = (-1) * (std::ceil(std::abs(minBox[i]) * fixedPoint16)) / fixedPoint16;
				if (maxBox[i] > 0)
					maxBox[i] = (std::ceil(maxBox[i] * fixedPoint16)) / fixedPoint16;
				else
					maxBox[i] = (-1) * (std::floor(std::abs(maxBox[i]) * fixedPoint16)) / fixedPoint16;
			}
		}

		glm::vec3 diag = maxBox - minBox;
		const float range = std::max(std::max(diag.x, diag.y), diag.z);
		const int32_t maxQuantizedValue = (1u << static_cast<uint32_t>(qp)) - 1;
		float scale = range / maxQuantizedValue;
		if (useFixedPoint)
			scale = (std::ceil(scale * fixedPoint16)) / fixedPoint16;

		std::cout << "  minPos=\"" << minBox.x << " " << minBox.y << " " << minBox.z << "\"" << std::endl;
		std::cout << "  maxPos=\"" << maxBox.x << " " << maxBox.y << " " << maxBox.z << "\"" << std::endl;
		std::cout << "  rangePos=" << range << std::endl;
		std::cout << "  maxQuantizedValue=" << maxQuantizedValue << std::endl;
		std::cout << "  scale=" << scale << std::endl;

		for (size_t i = 0; i < input.vertices.size() / 3; i++) {
			for (glm::vec3::length_type c = 0; c < 3; ++c) {
				//output.vertices[i * 3 + c] = (input.vertices[i * 3 + c] * range / maxQuantizedValue) + minBox[c];
				output.vertices[i * 3 + c] = (input.vertices[i * 3 + c] * scale) + minBox[c];
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
	if (!input.colors.empty() && ((qc >= 7) || (colorSpaceConversion) )) {
		const glm::vec3 minBox = minCol;
		const glm::vec3 maxBox = maxCol;
		const glm::vec3 diag = maxBox - minBox;
		const float range = std::max(std::max(diag.x, diag.y), diag.z);
		const int32_t maxQuantizedValue = (1u << static_cast<uint32_t>(qc)) - 1;

		std::cout << "  minCol=\"" << minBox.x << " " << minBox.y << " " << minBox.z << "\"" << std::endl;
		std::cout << "  maxCol=\"" << maxBox.x << " " << maxBox.y << " " << maxBox.z << "\"" << std::endl;
		std::cout << "  rangeCol=" << range << std::endl;

		for (size_t i = 0; i < input.colors.size() / 3; i++) {
			if(colorSpaceConversion){
				glm::vec3 inYUV;
				for (glm::vec3::length_type c = 0; c < 3; ++c) {
					inYUV[c] = (input.colors[i * 3 + c] / maxQuantizedValue);
				}
				output.colors[i * 3 + 0] = 255*(inYUV[0] + 1.57480 * (inYUV[2]-0.5));
				output.colors[i * 3 + 1] = 255*(inYUV[0] - 0.18733 * (inYUV[1]-0.5) - 0.46813 * (inYUV[2]-0.5));
				output.colors[i * 3 + 2] = 255*(inYUV[0] + 1.85563 * (inYUV[1]-0.5));
			}
			else{
				for (glm::vec3::length_type c = 0; c < 3; ++c) {
					output.colors[i * 3 + c] = (input.colors[i * 3 + c] * range / maxQuantizedValue) + minBox[c];
				}
			}
		}
	}
}
