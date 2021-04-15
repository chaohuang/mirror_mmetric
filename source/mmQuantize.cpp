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
			("minUv", "min corner of vertex texture coordinates bbox, a string of three floats. Computed of not set.",
				cxxopts::value<std::string>())
			("maxUv", "max corner of vertex texture coordinates bbox, a string of three floats. Computed of not set.",
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
			("useFixedPoint", "internally convert the minPos and maxPos to fixed point 16.",
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

		if (result.count("outputVar"))
			_outputVarFilename = result["outputVar"].as<std::string>();

		if (result.count("dequantize"))
			_dequantize = result["dequantize"].as<bool>();

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

bool Quantize::process(uint32_t frame) {

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

	std::cout << "Quantizing" << std::endl;
	std::cout << "  qp = " << _qp << std::endl;
	std::cout << "  qt = " << _qt << std::endl;
	std::cout << "  qn = " << _qn << std::endl;
	std::cout << "  qc = " << _qn << std::endl;

	if (_dequantize) {
		Model* quantizedModel = new Model();

		quantize(*inputModel, *quantizedModel, _qp, _qt, _qn, _qc,
			_outputVarFilename, _useFixedPoint,
			_minPos, _maxPos, _minUv, _maxUv, _minNrm, _maxNrm, _minCol, _maxCol);

		Dequantize::dequantize(*quantizedModel, *outputModel, _qp, _qt, _qn, _qc,
			_minPos, _maxPos, _minUv, _maxUv, _minNrm, _maxNrm, _minCol, _maxCol, _useFixedPoint);

		delete quantizedModel;
	}
	else {
		quantize(*inputModel, *outputModel, _qp, _qt, _qn, _qc,
			_outputVarFilename, _useFixedPoint,
			_minPos, _maxPos, _minUv, _maxUv, _minNrm, _maxNrm, _minCol, _maxCol);
	}

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

// input can be mesh or point cloud
// the min/max values are not set to const so that internally computed min/max
// will have the expected impact onto the --dequant option of the quantize method (see. Quantize::process).
void Quantize::quantize(
	const Model& input, Model& output,
	const uint32_t qp, const uint32_t qt, const uint32_t qn, const uint32_t qc,
	const std::string& outputVarFilename, const bool useFixedPoint,
	glm::vec3& minPos, glm::vec3& maxPos, glm::vec2& minUv, glm::vec2& maxUv,
	glm::vec3& minNrm, glm::vec3& maxNrm, glm::vec3& minCol, glm::vec3& maxCol
)
{
	// copy the input
	output = input;

	// prepare logging and output var
	std::vector<std::ostream*> out;
	out.push_back(&std::cout);
	std::ofstream fout;
	if (outputVarFilename != "") {
		fout.open(outputVarFilename.c_str(), std::ios::out);
		// this is mandatory to print floats with full precision
		fout.precision(std::numeric_limits< float >::max_digits10);
		if (fout) {
			// this is mandatory to print floats with full precision
			fout.precision(std::numeric_limits< float >::max_digits10);
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
			minPos = minBox;
			maxPos = maxBox;
		}
		else {
			std::cout << "Using parameter positions range" << std::endl;
			minBox = minPos; maxBox = maxPos;
		}
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
		const glm::vec3 diag = maxBox - minBox;
		const float range = std::max(std::max(diag.x, diag.y), diag.z);
		const int32_t maxPositionQuantizedValue = (1u << static_cast<uint32_t>(qp)) - 1;
		double scale = (double)range / maxPositionQuantizedValue;
		if (useFixedPoint)
			scale = (std::ceil(scale * fixedPoint16)) / fixedPoint16;

		for (size_t i = 0; i < out.size(); ++i) {
			*out[i] << "  minPos=\"" << minBox.x << " " << minBox.y << " " << minBox.z << "\"" << std::endl;
			*out[i] << "  maxPos=\"" << maxBox.x << " " << maxBox.y << " " << maxBox.z << "\"" << std::endl;
			*out[i] << "  rangePos=" << range << std::endl;
			*out[i] << "  maxPositionQuantizedValue=" << maxPositionQuantizedValue << std::endl;
			*out[i] << "  scale=" << scale << std::endl;
		}

		for (size_t i = 0; i < input.vertices.size() / 3; i++) {
			for (glm::vec3::length_type c = 0; c < 3; ++c) {
				uint32_t pos = static_cast<uint32_t> (std::floor(((double(input.vertices[i * 3 + c] - minBox[c])) / scale) + 0.5f));
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
			minUv = minBox;
			maxUv = maxBox;
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
			minNrm = minBox;
			maxNrm = maxBox;
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
			minCol = minBox;
			maxCol = maxBox;
		}
		else {
			std::cout << "Using parameter colors range" << std::endl;
			minBox = minCol; maxBox = maxCol;
		}
		const glm::vec3 diag = maxBox - minBox;
		const float range = std::max(std::max(diag.x, diag.y), diag.z);
		const int32_t maxColorQuantizedValue = (1u << static_cast<uint32_t>(qc)) - 1;

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
