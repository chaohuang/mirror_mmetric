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
// Author: meshToPcFace method based on original face sampling code from Owlii
// *****************************************************************

#include <iostream>
#include <fstream>
#include <set>
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
#include "mmSample.h"
#include "mmModel.h"
#include "mmImage.h"

// Descriptions of the command
const char* Sample::name = "sample";
const char* Sample::brief = "Convert mesh to point cloud";

//
Command* Sample::create() { return new Sample(); }

// 
bool Sample::initialize(Context* context, std::string app, int argc, char* argv[])
{
	_context = context;

	// command line parameters
	try
	{
		cxxopts::Options options(app + " " + name, brief);
		options.add_options()
			("i,inputModel", "path to input model (obj or ply file)",
				cxxopts::value<std::string>())
			("m,inputMap", "path to input texture map (png, jpg, rgb, yuv)",
				cxxopts::value<std::string>())
			("o,outputModel", "path to output model (obj or ply file)",
				cxxopts::value<std::string>())
			("mode", "the sampling mode in [face,grid,map,sdiv,ediv,prnd]",
				cxxopts::value<std::string>())
			("hideProgress", "hide progress display in console for use by robot",
				cxxopts::value<bool>()->default_value("false"))
			("outputCsv", "filename of the file where per frame statistics will append.",
				cxxopts::value<std::string>()->default_value(""))
			("h,help", "Print usage")
			;
		options.add_options("face mode")
			("float", "if set the processings and outputs will be float32, int32 otherwise",
				cxxopts::value<bool>()->default_value("true"))
			("thickness", "floating point value, distance to border of the face",
				cxxopts::value<float>()->default_value("0.0"))
			;
		options.add_options("sdiv mode")
			("areaThreshold", "face area limit to stop subdivision",
				cxxopts::value<float>()->default_value("1.0"))
			("mapThreshold", "if set will refine until face vertices texels are distanced of 1 and areaThreshold reached",
				cxxopts::value<bool>()->default_value("false"))
			;
		options.add_options("ediv mode")
			("lengthThreshold", "edge length limit to stop subdivision, used only if > 0, otherwise resolution is used.",
				cxxopts::value<float>()->default_value("0.0"))
			;
		options.add_options("grid mode")
			("gridSize", "integer value in [1,maxint], side size of the grid",
				cxxopts::value<int>()->default_value("1024"))
			("useNormal", "if set will sample only in the direction with the largest dot product with the triangle normal",
				cxxopts::value<bool>()->default_value("false"))
			("minPos", "min corner of vertex position bbox, a string of three floats.",
				cxxopts::value<std::string>())
			("maxPos", "max corner of vertex position bbox, a string of three floats.",
				cxxopts::value<std::string>())
			("useFixedPoint", "interprets minPos and maxPos inputs as fixed point 16.",
				cxxopts::value<bool>())
			;
		options.add_options("prnd mode")
			("nbSamples", "integer value specifying the traget number of points in the output point cloud",
				cxxopts::value<size_t>()->default_value("2000000"))
			;
		options.add_options("face and ediv modes")
			("resolution", "integer value in [1,maxuint], step/edgeLength = resolution / size(largest bbox side). In ediv mode, the resolution is used only if lengthThreshold=0.",
				cxxopts::value<size_t>()->default_value("1024"))
			;

		options.add_options("grid, face, sdiv and ediv modes.")
			("bilinear", "if set, texture filtering will be bilinear, nearest otherwise",
				cxxopts::value<bool>()->default_value("false"))
			("nbSamplesMin", "if set different from 0, the system will rerun the sampling multiple times to find the best parameter producing a number of samples in [nbAmplesMin, nbSamplesMax]. This process is very time comsuming.",
				cxxopts::value<size_t>()->default_value("0"))
			("nbSamplesMax", "see --nbSamplesMin documentation. Must be > to --nbSamplesMin.",
				cxxopts::value<size_t>()->default_value("0"))
			("maxIterations", "Maximum number of iterations in sample count constrained sampling, i.e. when --nbSampleMin > 0.",
				cxxopts::value<size_t>()->default_value("10"))
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
		if (result.count("inputMap"))
			inputTextureFilename = result["inputMap"].as<std::string>();
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

		if (mode != "face" && mode != "grid" && mode != "map" && 
			mode != "sdiv" && mode != "ediv" &&	mode != "prnd") {
			std::cerr << "Error: invalid mode \"" << mode << std::endl;
			return false;
		}

		//
		if (result.count("outputCsv"))
			_outputCsvFilename = result["outputCsv"].as<std::string>();

		//
		if (result.count("hideProgress"))
			hideProgress = result["hideProgress"].as<bool>();
		//
		if (result.count("resolution"))
			_resolution = result["resolution"].as<size_t>();
		if (result.count("thickness"))
			thickness = result["thickness"].as<float>();
		if (result.count("areaThreshold"))
			areaThreshold = result["areaThreshold"].as<float>();
		if (result.count("lengthThreshold"))
			lengthThreshold = result["lengthThreshold"].as<float>();
		if (result.count("mapThreshold"))
			mapThreshold = result["mapThreshold"].as<bool>();
		if (result.count("gridSize"))
			_gridSize = result["gridSize"].as<int>();
		if (result.count("useNormal"))
			_useNormal = result["useNormal"].as<bool>();
		if (result.count("bilinear"))
			bilinear = result["bilinear"].as<bool>();
		if (result.count("nbSamplesMin"))
			_nbSamplesMin = result["nbSamplesMin"].as<size_t>();
		if (result.count("nbSamplesMax")) {
			_nbSamplesMax = result["nbSamplesMax"].as<size_t>();
			if (_nbSamplesMax <= _nbSamplesMin) {
				std::cerr << "Error: nbSampleMax must be > to nbSampleMin" << std::endl;
				return false;
			}
		}
		if (result.count("maxIterations"))
			_maxIterations = result["maxIterations"].as<size_t>();

		// prnd options
		if (result.count("nbSamples"))
			_nbSamples = result["nbSamples"].as<size_t>();
			

		// grid options
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

bool Sample::process(uint32_t frame) {

	// Reading map if needed
	Image* textureMap;
	if (inputTextureFilename != "") {
		textureMap = IO::loadImage(inputTextureFilename);
	}
	else {
		std::cout << "Skipping map read, will parse use vertex color if any" << std::endl;
		textureMap = new Image();
	}

	// the input
	Model* inputModel;
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

	// create or open in append mode output csv if needed
	std::streamoff csvFileLength = 0;
	std::ofstream csvFileOut;
	if (_outputCsvFilename != "") {
		// check if csv file is empty, need to open in read mode
		std::ifstream filestr;
		filestr.open(_outputCsvFilename, std::ios::binary);
		if (filestr) {
			filestr.seekg(0, std::ios::end);
			csvFileLength = filestr.tellg();
			filestr.close();
		}
		// let's open in append write mode
		csvFileOut.open(_outputCsvFilename.c_str(), std::ios::out | std::ofstream::app);
		// this is mandatory to print floats with full precision
		csvFileOut.precision(std::numeric_limits< float >::max_digits10);
	}

	// Perform the processings
	clock_t t1 = clock();
	if (mode == "face") {
		std::cout << "Sampling in FACE mode" << std::endl;
		std::cout << "  Resolution = " << _resolution << std::endl;
		std::cout << "  Thickness = " << thickness << std::endl;
		std::cout << "  Bilinear = " << bilinear << std::endl;
		std::cout << "  hideProgress = " << hideProgress << std::endl;
		std::cout << "  nbSamplesMin = " << _nbSamplesMin << std::endl;
		std::cout << "  nbSamplesMax = " << _nbSamplesMax << std::endl;
		std::cout << "  maxIterations = " << _maxIterations << std::endl;
		size_t computedResolution = 0;
		if (_nbSamplesMin != 0) {
			std::cout << "  using contrained mode with nbSamples (costly!)" << std::endl;
			Sample::meshToPcFace(*inputModel, *outputModel, *textureMap,
				_nbSamplesMin, _nbSamplesMax, _maxIterations,
				thickness, bilinear, !hideProgress, computedResolution);
		}
		else {
			std::cout << "  using contrained mode with resolution " << std::endl;
			Sample::meshToPcFace(*inputModel, *outputModel, *textureMap,
				_resolution, thickness, bilinear, !hideProgress);
		}
		// print the stats
		if (csvFileOut) {
			// print the header if file is empty
			if (csvFileLength == 0) {
				csvFileOut << "model;texture;frame;resolution;thickness;bilinear;nbSamplesMin;"
					<< "nbSamplesMax;maxIterations;computedResolution;nbSamples" << std::endl;
			}
			// print stats
			csvFileOut << inputModelFilename << ";" << inputTextureFilename << ";" << frame << ";" << _resolution << ";"
				<< thickness << ";" << bilinear << ";" << _nbSamplesMin
				<< ";" << _nbSamplesMax << ";" << _maxIterations << ";" << computedResolution << ";"
				<< outputModel->getPositionCount() << std::endl;
			// done
			csvFileOut.close();
		}
	}
	else if (mode == "grid") {
		std::cout << "Sampling in GRID mode" << std::endl;
		std::cout << "  Grid Size = " << _gridSize << std::endl;
		std::cout << "  Use Normal = " << _useNormal << std::endl;
		std::cout << "  Bilinear = " << bilinear << std::endl;
		std::cout << "  hideProgress = " << hideProgress << std::endl;
		std::cout << "  nbSamplesMin = " << _nbSamplesMin << std::endl;
		std::cout << "  nbSamplesMax = " << _nbSamplesMax << std::endl;
		std::cout << "  maxIterations = " << _maxIterations << std::endl;
		size_t computedResolution = 0;
		if (_nbSamplesMin != 0) {
			std::cout << "  using contrained mode with nbSamples (costly!)" << std::endl;
			Sample::meshToPcGrid(*inputModel, *outputModel, *textureMap,
				_nbSamplesMin, _nbSamplesMax, _maxIterations,
				bilinear, !hideProgress, _useNormal, _useFixedPoint,
				_minPos, _maxPos, computedResolution);
		}
		else {
			std::cout << "  using contrained mode with gridSize " << std::endl;
			Sample::meshToPcGrid(*inputModel, *outputModel,
				*textureMap, _gridSize, bilinear, !hideProgress, _useNormal, _useFixedPoint, _minPos, _maxPos);
		}
		// print the stats
		if (csvFileOut) {
			// print the header if file is empty
			if (csvFileLength == 0) {
				csvFileOut << "model;texture;frame;mode;gridSize;useNormal;bilinear;nbSamplesMin;"
					<< "nbSamplesMax;maxIterations;computedResolution;nbSamples" << std::endl;
			}
			// print stats
			csvFileOut << inputModelFilename << ";" << inputTextureFilename << ";" << frame << ";" << mode << ";" 
				<< _gridSize << ";" << _useNormal << ";" << bilinear << ";"
				<< _nbSamplesMin << ";" << _nbSamplesMax << ";" << _maxIterations << ";" << computedResolution << ";"
				<< outputModel->getPositionCount() << std::endl;
			// done
			csvFileOut.close();
		}
	}
	else if (mode == "map") {
		std::cout << "Sampling in MAP mode" << std::endl;
		std::cout << "  hideProgress = " << hideProgress << std::endl;
		Sample::meshToPcMap(*inputModel, *outputModel, *textureMap, !hideProgress);
		// print the stats
		if (csvFileOut) {
			// print the header if file is empty
			if (csvFileLength == 0) {
				csvFileOut << "model;texture;frame;mode;nbSamples" << std::endl;
			}
			// print stats
			csvFileOut << inputModelFilename << ";" << inputTextureFilename << ";" << frame << ";" << mode << ";"
				<< outputModel->getPositionCount() << std::endl;
			// done
			csvFileOut.close();
		}
	}
	else if (mode == "sdiv") {
		std::cout << "Sampling in SDIV mode" << std::endl;
		std::cout << "  Area threshold = " << areaThreshold << std::endl;
		std::cout << "  Map threshold = " << mapThreshold << std::endl;
		std::cout << "  Bilinear = " << bilinear << std::endl;
		std::cout << "  hideProgress = " << hideProgress << std::endl;
		std::cout << "  nbSamplesMin = " << _nbSamplesMin << std::endl;
		std::cout << "  nbSamplesMax = " << _nbSamplesMax << std::endl;
		std::cout << "  maxIterations = " << _maxIterations << std::endl;
		float computedThres = 0.0f;
		if (_nbSamplesMin != 0) {
			std::cout << "  using contrained mode with nbSamples (costly!)" << std::endl;
			Sample::meshToPcDiv(*inputModel, *outputModel, *textureMap,
				_nbSamplesMin, _nbSamplesMax, _maxIterations,
				bilinear, !hideProgress, computedThres);
		}
		else {
			Sample::meshToPcDiv(*inputModel, *outputModel, *textureMap,
				areaThreshold, mapThreshold, bilinear, !hideProgress);
		}
		// print the stats
		if (csvFileOut) {
			// print the header if file is empty
			if (csvFileLength == 0) {
				csvFileOut << "model;texture;frame;mode;areaThreshold;bilinear;nbSamplesMin;"
					<< "nbSamplesMax;maxIterations;computedThreshold;nbSamples" << std::endl;
			}
			// print stats
			csvFileOut << inputModelFilename << ";" << inputTextureFilename << ";" << frame << ";" << mode << ";"
				<< areaThreshold << ";" << bilinear << ";"
				<< _nbSamplesMin << ";" << _nbSamplesMax << ";" << _maxIterations << ";" << computedThres << ";"
				<< outputModel->getPositionCount() << std::endl;
			// done
			csvFileOut.close();
		}
	}
	else if (mode == "ediv") {
		std::cout << "Sampling in EDIV mode" << std::endl;
		std::cout << "  Edge length threshold = " << lengthThreshold << std::endl;
		std::cout << "  Resolution = " << _resolution << (lengthThreshold != 0 ? "(skipped)" : "") << std::endl;
		std::cout << "  Bilinear = " << bilinear << std::endl;
		std::cout << "  hideProgress = " << hideProgress << std::endl;
		std::cout << "  nbSamplesMin = " << _nbSamplesMin << std::endl;
		std::cout << "  nbSamplesMax = " << _nbSamplesMax << std::endl;
		std::cout << "  maxIterations = " << _maxIterations << std::endl;
		float computedThres = 0.0f;
		if (_nbSamplesMin != 0) {
			std::cout << "  using contrained mode with nbSamples (costly!)" << std::endl;
			Sample::meshToPcDivEdge(*inputModel, *outputModel, *textureMap,
				_nbSamplesMin, _nbSamplesMax, _maxIterations,
				bilinear, !hideProgress, computedThres);
		}
		else {
			Sample::meshToPcDivEdge(*inputModel, *outputModel, *textureMap,
				lengthThreshold, _resolution, bilinear, !hideProgress, computedThres);
		}
		// print the stats
		if (csvFileOut) {
			// print the header if file is empty
			if (csvFileLength == 0) {
				csvFileOut << "model;texture;frame;mode;resolution;lengthThreshold;bilinear;nbSamplesMin;"
					<< "nbSamplesMax;maxIterations;computedThreshold;nbSamples" << std::endl;
			}
			// print stats
			csvFileOut << inputModelFilename << ";" << inputTextureFilename << ";" << frame << ";" << mode << ";"
				<< _resolution << ";" << lengthThreshold << ";" << bilinear << ";"
				<< _nbSamplesMin << ";" << _nbSamplesMax << ";" << _maxIterations << ";" << computedThres << ";"
				<< outputModel->getPositionCount() << std::endl;
			// done
			csvFileOut.close();
		}
	}
	else if (mode == "prnd") {
		std::cout << "Sampling in PRND mode" << std::endl;
		std::cout << "  nbSamples = " << _nbSamples << std::endl;
		std::cout << "  Bilinear = " << bilinear << std::endl;
		std::cout << "  hideProgress = " << hideProgress << std::endl;
		Sample::meshToPcPrnd(*inputModel, *outputModel, *textureMap, _nbSamples,
			bilinear, !hideProgress);
		// print the stats
		if (csvFileOut) {
			// print the header if file is empty
			if (csvFileLength == 0) {
				csvFileOut << "model;texture;frame;mode;targetPointCount;bilinear;nbSamples" << std::endl;
			}
			// print stats
			csvFileOut << inputModelFilename << ";" << inputTextureFilename << ";" << frame << ";" << mode << ";"
				<< _nbSamples << ";" << bilinear << ";" << outputModel->getPositionCount() << std::endl;
			// done
			csvFileOut.close();
		}
	}
	clock_t t2 = clock();
	std::cout << "Time on processing: " << ((float)(t2 - t1)) / CLOCKS_PER_SEC << " sec." << std::endl;

	// save the result
	if (IO::saveModel(outputModelFilename, outputModel))
		return true;
	else
		return false;

}

// this algorithm was originally developped by Owlii
void Sample::meshToPcFace(
	const Model& input, Model& output,
	const Image& tex_map, size_t resolution, float thickness, bool bilinear, bool logProgress) {

	// computes the bounding box of the vertices
	glm::vec3 minPos, maxPos;
	computeBBox(input.vertices, minPos, maxPos);
	std::cout << "minbox = " << minPos[0] << "," << minPos[1] << "," << minPos[2] << std::endl;
	std::cout << "maxbox = " << maxPos[0] << "," << maxPos[1] << "," << maxPos[2] << std::endl;

	// computes the sampling step
	glm::vec3 diag = maxPos - minPos;
	float boxMaxSize = std::max(diag.x, std::max(diag.y, diag.z));
	float step = boxMaxSize / resolution;
	std::cout << "step = " << step << std::endl;

	// to prevent storing duplicate points, we use a ModelBuilder
	ModelBuilder builder(output);

	size_t skipped = 0; // number of degenerate triangles

	for (size_t t = 0; t < input.triangles.size() / 3; t++) {

		if (logProgress)
			std::cout << '\r' << t << "/" << input.triangles.size() / 3 << std::flush;

		Vertex v1, v2, v3;

		fetchTriangle(input, t,
			input.uvcoords.size() != 0,
			input.colors.size() != 0,
			input.normals.size() != 0,
			v1, v2, v3);

		// check if triangle is not degenerate
		if (triangleArea(v1.pos, v2.pos, v3.pos) < DBL_EPSILON) {
			++skipped;
			continue;
		}

		// compute face normal
		glm::vec3 normal;
		triangleNormal(v1.pos, v2.pos, v3.pos, normal);

		// computes face dimensions for sampling
		glm::vec3 v12_norm = v2.pos - v1.pos;
		glm::vec3 v23_norm = v3.pos - v2.pos;
		float l12 = glm::length(v12_norm);
		float l23 = glm::length(v23_norm);
		for (int i = 0; i < 3; i++) {
			v12_norm[i] = v12_norm[i] / l12;
			v23_norm[i] = v23_norm[i] / l23;
		}

		// do the sampling
		for (float step12 = 0.f; step12 <= l12; step12 += step) {
			for (float step23 = 0.f; step23 <= step12 / l12 * l23; step23 += step) {

				float step_normal_bdry = 0.f;
				while (step_normal_bdry <= thickness) {
					step_normal_bdry += step;
				}
				step_normal_bdry -= step;

				for (float step_normal = -step_normal_bdry; step_normal <= step_normal_bdry; step_normal += step) {

					Vertex v;
					v.pos = v1.pos + step12 * v12_norm + step23 * v23_norm + step_normal * normal;
					v.nrm = normal; v.hasNormal = true;

					// compute the color if any
					if (input.uvcoords.size() != 0 && tex_map.data != NULL) { // use the texture map
						// compute UV
						const glm::vec2 uv{
							(v1.uv[0] + step12 / l12 * (v2.uv[0] - v1.uv[0]) + step23 / l23 * (v3.uv[0] - v2.uv[0])),
							(v1.uv[1] + step12 / l12 * (v2.uv[1] - v1.uv[1]) + step23 / l23 * (v3.uv[1] - v2.uv[1]))
						};

						// fetch the color from the map
						if (bilinear)
							texture2D_bilinear(tex_map, uv, v.col);
						else
							texture2D(tex_map, uv, v.col);
						v.hasColor = true;
					}
					else if (input.colors.size() != 0) { // use color per vertex
						v.col[0] = v1.col[0] + step12 / l12 * (v2.col[0] - v1.col[0]) + step23 / l23 * (v3.col[0] - v2.col[0]);
						v.col[1] = v1.col[1] + step12 / l12 * (v2.col[1] - v1.col[1]) + step23 / l23 * (v3.col[1] - v2.col[1]);
						v.col[2] = v1.col[2] + step12 / l12 * (v2.col[2] - v1.col[2]) + step23 / l23 * (v3.col[2] - v2.col[2]);
						v.hasColor = true;
					}

					// add the vertex 
					builder.pushVertex(v);
				}
			}
		}
	}
	if (logProgress)
		std::cout << std::endl;
	if (skipped != 0)
		std::cout << "Skipped " << skipped << " degenerate triangles" << std::endl;
	if (builder.foundCount != 0)
		std::cout << "Skipped " << builder.foundCount << " duplicate vertices" << std::endl;
	std::cout << "Generated " << output.vertices.size() / 3 << " points" << std::endl;
}

void Sample::meshToPcFace(
	const Model& input, Model& output, const Image& tex_map,
	size_t nbSamplesMin, size_t nbSamplesMax, size_t maxIterations,
	float thickness, bool bilinear, bool logProgress, size_t& computedResolution)
{
	size_t resolution = 1024;
	meshToPcFace(input, output, tex_map, resolution, thickness, bilinear, logProgress);
	// search to init the algo bounds
	size_t minResolution = 0;
	size_t maxResolution = 0;
	//
	size_t iter = 0;
	while ((output.getPositionCount() < nbSamplesMin || output.getPositionCount() > nbSamplesMax) && iter < maxIterations) {
		iter++;
		// let's refine
		if (output.getPositionCount() < nbSamplesMin) { // need to add some points
			minResolution = resolution;
			if (maxResolution == 0) {
				resolution = minResolution * 2;
			}
			else {
				resolution = minResolution + (maxResolution - minResolution) / 2;
			}
		}
		if (output.getPositionCount() > nbSamplesMax) { // need to remove some points
			maxResolution = resolution;
			resolution = minResolution + (maxResolution - minResolution) / 2;
		}
		std::cout << "  posCount=" << output.getPositionCount() << std::endl;
		std::cout << "  minResolution=" << minResolution << std::endl;
		std::cout << "  maxResolution=" << maxResolution << std::endl;
		std::cout << "  resolution=" << resolution << std::endl;
		//
		output.reset();
		meshToPcFace(input, output, tex_map, resolution, thickness, bilinear, logProgress);
	}
	computedResolution = resolution;
	std::cout << "algorithm ended after " << iter << " iterations " << std::endl;
}

// we use ray tracing to process the result, we could also use a rasterization (might be faster)
void Sample::meshToPcGrid(
	const Model& input, Model& output,
	const Image& tex_map, const size_t resolution, const bool bilinear, const bool logProgress, 
	bool useNormal, bool useFixedPoint, glm::vec3& minPos, glm::vec3& maxPos) 
{

	// computes the bounding box of the vertices
	glm::vec3 minBox = minPos;
	glm::vec3 maxBox = maxPos;
	const int32_t fixedPoint16 = (1u << 16);
	glm::vec3 stepSize;
	if (minPos == maxPos) {
		std::cout << "Computing positions range" << std::endl;
		computeBBox(input.vertices, minBox, maxBox);
		std::cout << "minBox = " << minBox[0] << "," << minBox[1] << "," << minBox[2] << std::endl;
		std::cout << "maxBox = " << maxBox[0] << "," << maxBox[1] << "," << maxBox[2] << std::endl;
		std::cout << "Transform bounding box to square box" << std::endl;
		// hence sampling will be unform in the three dimensions
		toCubicalBBox(minBox, maxBox); // this will change the origin of the coordinate system (but it is just a translation)
		stepSize = (maxBox - minBox) * (1.0F / (float)(resolution - 1));
	}
	else {
		std::cout << "Using parameter positions range" << std::endl;
		std::cout << "minPos = " << minPos[0] << "," << minPos[1] << "," << minPos[2] << std::endl;
		std::cout << "maxPos = " << maxPos[0] << "," << maxPos[1] << "," << maxPos[2] << std::endl;
		if (useFixedPoint) {
			// converting the values to a fixed point representation
			// minBox(FP16) will be used in AAPS -> shift
			for (int i = 0; i < 3; i++) {
				if (minBox[i] > 0)
					minBox[i] = (std::floor(minBox[i] * fixedPoint16)) / fixedPoint16;
				else
					minBox[i] = (-1) * (std::ceil(std::abs(minBox[i]) * fixedPoint16)) / fixedPoint16;
				if (maxBox[i] > 0){
					maxBox[i] = std::ceil(maxPos[i] * fixedPoint16) / fixedPoint16;
				}
				else
					maxBox[i] = (-1) * (std::floor(std::abs(maxBox[i]) * fixedPoint16)) / fixedPoint16;
			}
		}
		std::cout << "minBox = " << minBox[0] << "," << minBox[1] << "," << minBox[2] << std::endl;
		std::cout << "maxBox = " << maxBox[0] << "," << maxBox[1] << "," << maxBox[2] << std::endl;
		const glm::vec3 diag = maxBox - minBox;
		float range = std::max(std::max(diag.x, diag.y), diag.z);
		stepSize[0] = range * (1.0F / (float)(resolution - 1));
		stepSize[1] = range * (1.0F / (float)(resolution - 1));
		stepSize[2] = range * (1.0F / (float)(resolution - 1));
		if (useFixedPoint){
			stepSize[0] = (std::ceil(stepSize[0] * fixedPoint16)) / fixedPoint16;
			stepSize[1] = (std::ceil(stepSize[1] * fixedPoint16)) / fixedPoint16;
			stepSize[2] = (std::ceil(stepSize[2] * fixedPoint16)) / fixedPoint16;
		}
	}

	std::cout << "stepSize = " << stepSize[0] << "," << stepSize[1] << "," << stepSize[2] << std::endl;

	// we will now sample between min and max over the three dimensions, using resolution
	// by throwing rays from the three orthogonal faces of the box XY, XZ, YZ

	// to prevent storing duplicate points, we use a ModelBuilder
	ModelBuilder builder(output);

	size_t skipped = 0; // number of degenerate triangles

	// for each triangle
	for (size_t triIdx = 0; triIdx < input.triangles.size() / 3; ++triIdx) {

		if (logProgress)
			std::cout << '\r' << triIdx << "/" << input.triangles.size() / 3 << std::flush;

		Vertex v1, v2, v3;

		fetchTriangle(input, triIdx,
			input.uvcoords.size() != 0,
			input.colors.size() != 0,
			input.normals.size() != 0,
			v1, v2, v3);

		// check if triangle is not degenerate
		if (triangleArea(v1.pos, v2.pos, v3.pos) < DBL_EPSILON) {
			++skipped;
			continue;
		}

		// compute face normal
		glm::vec3 normal;
		triangleNormal(v1.pos, v2.pos, v3.pos, normal);

		// extract the triangle bbox
		glm::vec3 triMinBox, triMaxBox;
		triangleBBox(v1.pos, v2.pos, v3.pos, triMinBox, triMaxBox);

		// now find the Discrete range from global box to triangle box
		glm::vec3 lmin = glm::floor((triMinBox - minBox) / stepSize); // can lead to division by zero with flat box, handled later
		glm::vec3 lmax = glm::ceil((triMaxBox - minBox) / stepSize); // idem
		glm::vec3 lcnt = lmax - lmin;
		
		// now we will send rays on this triangle from the discreet steps of this box
		// rayTrace from the three main axis

		// reordering the search to start with the direction closest to the triangle normal
		glm::ivec3 mainAxisVector(0,1,2);
		// we want to preserve invariance with existing references if useNormal is disabled
		// so we do the following reordering only if option is enabled
		if (useNormal) {
			if ((std::abs(normal[0]) >= std::abs(normal[1])) && (std::abs(normal[0]) >= std::abs(normal[2]))) {
				if (std::abs(normal[1]) >= std::abs(normal[2]))
					mainAxisVector = glm::ivec3(0, 1, 2);
				else
					mainAxisVector = glm::ivec3(0, 2, 1);
			}
			else {
				if ((std::abs(normal[1]) >= std::abs(normal[0])) && (std::abs(normal[1]) >= std::abs(normal[2]))) {
					if (std::abs(normal[0]) >= std::abs(normal[2]))
						mainAxisVector = glm::ivec3(1, 0, 2);
					else
						mainAxisVector = glm::ivec3(1, 2, 0);
				}
				else {
					if (std::abs(normal[0]) >= std::abs(normal[1]))
						mainAxisVector = glm::ivec3(2, 0, 1);
					else
						mainAxisVector = glm::ivec3(2, 1, 0);
				}
			}
		}
		int mainAxisMaxIndex = useNormal ? 1 : 3; // if useNormal is selected, we only need to check the first index

		for (int mainAxisIndex = 0; mainAxisIndex < mainAxisMaxIndex; ++mainAxisIndex) {

			glm::vec3::length_type mainAxis = mainAxisVector[mainAxisIndex];
			// axis swizzling
			glm::vec3::length_type secondAxis = 1;	glm::vec3::length_type thirdAxis = 2;
			if (mainAxis == 1) { secondAxis = 0; thirdAxis = 2; }
			else if (mainAxis == 2) { secondAxis = 0; thirdAxis = 1; }

			// skip this axis if box is null sized on one of the two other axis
			if (minBox[secondAxis] == maxBox[secondAxis] || minBox[thirdAxis] == maxBox[thirdAxis])
				continue;

			// let's throw from mainAxis prependicular plane
			glm::vec3 rayOrigin = { 0.0,0.0,0.0 };
			glm::vec3 rayDirection = { 0.0,0.0,0.0 };

			// on the main axis
			if (stepSize[mainAxis] == 0.0F) { // handle stepSize[axis]==0
				// add small thress to be sure ray intersect in positive t
				rayOrigin[mainAxis] = minBox[mainAxis] - 0.5F;
			}
			else {
				rayOrigin[mainAxis] = minBox[mainAxis] + lmin[mainAxis] * stepSize[mainAxis];
			}
			// on main axis from min to max
			rayDirection[mainAxis] = 1.0;

			// iterate the second axis with i 
			for (size_t i = 0; i <= lcnt[secondAxis]; ++i) {

				// iterate the third axis with j
				for (size_t j = 0; j <= lcnt[thirdAxis]; ++j) {

					// create the ray, starting from the face of the triangle bbox
					rayOrigin[secondAxis] = minBox[secondAxis] + (lmin[secondAxis] + i) * stepSize[secondAxis];
					rayOrigin[thirdAxis] = minBox[thirdAxis] + (lmin[thirdAxis] + j) * stepSize[thirdAxis];

					//  triplet, x = t, y = u, z = v with t the parametric and (u,v) the barycentrics
					glm::vec3 res;

					// let' throw the ray toward the triangle
					if (evalRayTriangle(rayOrigin, rayDirection, v1.pos, v2.pos, v3.pos, res)) {

						// we convert the result into a point with color
						Vertex v;
						v.pos = rayOrigin + rayDirection * res[0];
						v.nrm = normal; v.hasNormal = true;

						// compute the color fi any
						// use the texture map
						if (input.uvcoords.size() != 0 && tex_map.data != NULL) {

							// use barycentric coordinates to extract point UV
							glm::vec2 uv = v1.uv * (1.0f - res.y - res.z) + v2.uv * res.y + v3.uv * res.z;

							// fetch the color from the map
							if (bilinear)
								texture2D_bilinear(tex_map, uv, v.col);
							else
								texture2D(tex_map, uv, v.col);

							v.hasColor = true;
							//v.col = v.col * rayDirection; --> for debugging, paints the color of the vertex according to the direction
						}
						// use color per vertex
						else if (input.colors.size() != 0) {
							// compute pixel color using barycentric coordinates
							v.col = v1.col * (1.0f - res.y - res.z) + v2.col * res.y + v3.col * res.z;
							v.hasColor = true;
						}

						// add the vertex 
						builder.pushVertex(v);
					}
				}
			}
		}
	}
	if (logProgress)
		std::cout << std::endl;
	if (skipped != 0)
		std::cout << "Skipped " << skipped << " degenerate triangles" << std::endl;
	if (builder.foundCount != 0)
		std::cout << "Skipped " << builder.foundCount << " duplicate vertices" << std::endl;
	std::cout << "Generated " << output.vertices.size() / 3 << " points" << std::endl;

}

void Sample::meshToPcGrid(
	const Model& input, Model& output, const Image& tex_map,
	size_t nbSamplesMin, size_t nbSamplesMax, size_t maxIterations,
	bool bilinear, bool logProgress, bool useNormal, 
	bool useFixedPoint, glm::vec3& minPos, glm::vec3& maxPos, size_t& computedResolution)
{
	size_t resolution = 1024;
	meshToPcGrid(input, output, tex_map, resolution, bilinear, logProgress, useNormal, useFixedPoint, minPos, maxPos);
	// search to init the algo bounds
	size_t minResolution = 0;
	size_t maxResolution = 0;
	//
	size_t iter = 0;
	while ((output.getPositionCount() < nbSamplesMin || output.getPositionCount() > nbSamplesMax) && iter < maxIterations) {
		iter++;
		// let's refine
		if (output.getPositionCount() < nbSamplesMin) { // need to add some points
			minResolution = resolution;
			if (maxResolution == 0) {
				resolution = minResolution * 2;
			}
			else {
				resolution = minResolution + (maxResolution - minResolution) / 2;
			}
		}
		if (output.getPositionCount() > nbSamplesMax) { // need to remove some points
			maxResolution = resolution;
			resolution = minResolution + (maxResolution - minResolution) / 2;
		}
		std::cout << "  posCount=" << output.getPositionCount() << std::endl;
		std::cout << "  minResolution=" << minResolution << std::endl;
		std::cout << "  maxResolution=" << maxResolution << std::endl;
		std::cout << "  resolution=" << resolution << std::endl;
		//
		output.reset();
		meshToPcGrid(input, output, tex_map, resolution, bilinear, logProgress, useNormal, useFixedPoint, minPos, maxPos);
	}
	computedResolution = resolution;
	std::cout << "algorithm ended after " << iter << " iterations " << std::endl;
}


// perform a reverse sampling of the texture map to generate mesh samples
// the color of the point is then using the texel color => no filtering
void Sample::meshToPcMap(
	const Model& input, Model& output,
	const Image& tex_map, bool logProgress) {

	if (input.uvcoords.size() == 0) {
		std::cerr << "Error: cannot back sample model, no UV coordinates" << std::endl;
		return;
	}
	if (tex_map.width <= 0 || tex_map.height <= 0 || tex_map.nbc < 3 || tex_map.data == NULL) {
		std::cerr << "Error: cannot back sample model, no valid texture map" << std::endl;
		return;
	}

	// to prevent storing duplicate points, we use a ModelBuilder
	ModelBuilder builder(output);

	size_t skipped = 0; // number of degenerate triangles

	// For each triangle
	for (size_t triIdx = 0; triIdx < input.triangles.size() / 3; ++triIdx) {

		if (logProgress)
			std::cout << '\r' << triIdx << "/" << input.triangles.size() / 3 << std::flush;

		Vertex v1, v2, v3;

		fetchTriangle(input, triIdx,
			input.uvcoords.size() != 0,
			input.colors.size() != 0,
			input.normals.size() != 0,
			v1, v2, v3);

		// check if triangle is not degenerate
		if (triangleArea(v1.pos, v2.pos, v3.pos) < DBL_EPSILON) {
			++skipped;
			continue;
		}

		// compute face normal
		glm::vec3 normal;
		triangleNormal(v1.pos, v2.pos, v3.pos, normal);

		// compute the UVs bounding box
		glm::vec2 uvMin = { FLT_MAX, FLT_MAX };
		glm::vec2 uvMax = { -FLT_MAX, -FLT_MAX };
		uvMin = glm::min(v3.uv, glm::min(v2.uv, glm::min(v1.uv, uvMin)));
		uvMax = glm::max(v3.uv, glm::max(v2.uv, glm::max(v1.uv, uvMax)));

		// find the integer coordinates covered in the map
		glm::i32vec2 intUvMin = { (tex_map.width - 1) * uvMin.x, (tex_map.height - 1) * uvMin.y };
		glm::i32vec2 intUvMax = { (tex_map.width - 1) * uvMax.x, (tex_map.height - 1) * uvMax.y };

		// loop over the box in image space
		// if a pixel center is in the triangle then backproject
		// and create a new point with the pixel color
		for (size_t i = intUvMin[0]; i <= intUvMax[0]; ++i) {
			for (size_t j = intUvMin[1]; j <= intUvMax[1]; ++j) {

				// the new vertex
				Vertex v;
				// force to face normal
				v.hasNormal = true; v.nrm = normal;
				// get the UV for the center of the pixel
				v.hasUVCoord = true; v.uv = { (0.5F + i) / tex_map.width, (0.5F + j) / tex_map.height };

				// test if this pixelUV is in the triangle UVs
				glm::vec3 bary; // the barycentrics if success
				if (getBarycentric(v.uv, v1.uv, v2.uv, v3.uv, bary)) {

					// revert pixelUV to find point in 3D
					triangleInterpolation(v1.pos, v2.pos, v3.pos, bary.x, bary.y, v.pos);

					// fetch the color
					texture2D(tex_map, v.uv, v.col);
					v.hasColor = true;

					// add to results
					builder.pushVertex(v);
				}
			}
		}
	}
	if (logProgress)
		std::cout << std::endl;
	if (skipped != 0)
		std::cout << "Skipped " << skipped << " degenerate triangles" << std::endl;
	if (builder.foundCount != 0)
		std::cout << "Skipped " << builder.foundCount << " duplicate vertices" << std::endl;
	std::cout << "Generated " << output.vertices.size() / 3 << " points" << std::endl;
}

// recursive body of meshtoPvDiv
void subdivideTriangle(
	const Vertex& v1,
	const Vertex& v2,
	const Vertex& v3,
	const Image& tex_map,
	const float thres,
	const bool mapThreshold,
	const bool bilinear,
	ModelBuilder& output
) {

	// recursion stop criterion on area
	bool areaReached = triangleArea(v1.pos, v2.pos, v3.pos) < thres;

	// recursion stop criterion on texels adjacency
	if (mapThreshold && tex_map.data != NULL) {
		const glm::ivec2 mapSize = { tex_map.width, tex_map.height };
		glm::ivec2 mapCoord1, mapCoord2, mapCoord3;
		mapCoordClamped(v1.uv, mapSize, mapCoord1);
		mapCoordClamped(v2.uv, mapSize, mapCoord2);
		mapCoordClamped(v3.uv, mapSize, mapCoord3);
		if (std::abs(mapCoord1.x - mapCoord2.x) <= 1 &&
			std::abs(mapCoord1.x - mapCoord3.x) <= 1 &&
			std::abs(mapCoord2.x - mapCoord3.x) <= 1 &&
			std::abs(mapCoord1.y - mapCoord2.y) <= 1 &&
			std::abs(mapCoord1.y - mapCoord3.y) <= 1 &&
			std::abs(mapCoord2.y - mapCoord3.y) <= 1 &&
			areaReached)
		{
			return;
		}
	}
	else if (areaReached) {
		return;
	}

	//
	glm::vec3 normal;
	triangleNormal(v1.pos, v2.pos, v3.pos, normal);

	// the new vertices
	Vertex e1, e2, e3;
	// we sue v1 as reference in term of components to push
	e1.hasColor = e2.hasColor = e3.hasColor = v1.hasColor;
	e1.hasUVCoord = e2.hasUVCoord = e3.hasUVCoord = v1.hasUVCoord;
	// forces normals, we use generated per face ones - might be better as an option
	e1.hasNormal = e2.hasNormal = e3.hasNormal = true;

	// edge centers 
	// (we do not interpolate normals but use face normal) - might be better as an option
	e1.pos = v1.pos * 0.5F + v2.pos * 0.5F;
	e1.col = v1.col * 0.5F + v2.col * 0.5F;
	e1.uv = v1.uv * 0.5F + v2.uv * 0.5F;
	e1.nrm = normal;

	e2.pos = v2.pos * 0.5F + v3.pos * 0.5F;
	e2.col = v2.col * 0.5F + v3.col * 0.5F;
	e2.uv = v2.uv * 0.5F + v3.uv * 0.5F;
	e2.nrm = normal;

	e3.pos = v3.pos * 0.5F + v1.pos * 0.5F;
	e3.col = v3.col * 0.5F + v1.col * 0.5F;
	e3.uv = v3.uv * 0.5F + v1.uv * 0.5F;
	e3.nrm = normal;

	// push the new vertices
	output.pushVertex(e1, tex_map, bilinear);
	output.pushVertex(e2, tex_map, bilinear);
	output.pushVertex(e3, tex_map, bilinear);

	// go deeper in the subdivision
	subdivideTriangle(e1, e2, e3, tex_map, thres, mapThreshold, bilinear, output);
	subdivideTriangle(v1, e1, e3, tex_map, thres, mapThreshold, bilinear, output);
	subdivideTriangle(e1, v2, e2, tex_map, thres, mapThreshold, bilinear, output);
	subdivideTriangle(e2, v3, e3, tex_map, thres, mapThreshold, bilinear, output);
}

// Use triangle subdivision algorithm to perform the sampling
// Use simple subdiv scheme, stop criterion on triangle area or texture sample distance <= 1 pixel
void Sample::meshToPcDiv(
	const Model& input, Model& output,
	const Image& tex_map, float areaThreshold, bool mapThreshold, bool bilinear, bool logProgress)
{
	// number of degenerate triangles
	size_t skipped = 0;

	// to prevent storing duplicate points, we use a ModelBuilder
	ModelBuilder builder(output);

	// For each triangle
	for (size_t triIdx = 0; triIdx < input.triangles.size() / 3; ++triIdx) {

		if (logProgress)
			std::cout << '\r' << triIdx << "/" << input.triangles.size() / 3 << std::flush;

		Vertex v1, v2, v3;

		fetchTriangle(input, triIdx,
			input.uvcoords.size() != 0,
			input.colors.size() != 0,
			input.normals.size() != 0,
			v1, v2, v3);

		// check if triangle is not degenerate
		if (triangleArea(v1.pos, v2.pos, v3.pos) < DBL_EPSILON) {
			++skipped;
			continue;
		}

		// compute face normal (forces) - might be better as an option
		glm::vec3 normal;
		triangleNormal(v1.pos, v2.pos, v3.pos, normal);
		v1.nrm = v2.nrm = v3.nrm = normal;
		v1.hasNormal = v2.hasNormal = v3.hasNormal = true;

		// push the vertices
		builder.pushVertex(v1, tex_map, bilinear);
		builder.pushVertex(v2, tex_map, bilinear);
		builder.pushVertex(v3, tex_map, bilinear);

		// subdivide recursively
		subdivideTriangle(v1, v2, v3, tex_map, areaThreshold, mapThreshold, bilinear, builder);

	}
	if (logProgress)
		std::cout << std::endl;
	if (skipped != 0)
		std::cout << "Skipped " << skipped << " degenerate triangles" << std::endl;
	if (builder.foundCount != 0)
		std::cout << "Handled " << builder.foundCount << " duplicate vertices" << std::endl;
	std::cout << "Generated " << output.vertices.size() / 3 << " points" << std::endl;

}

void Sample::meshToPcDiv(
	const Model& input, Model& output, const Image& tex_map,
	size_t nbSamplesMin, size_t nbSamplesMax, size_t maxIterations,
	bool bilinear, bool logProgress, float& computedThres)
{
	float value = 1.0;
	meshToPcDiv(input, output, tex_map, value, 0, bilinear, logProgress);
	// search to init the algo bounds
	float minBound = 0;
	float maxBound = 0;
	//
	size_t iter = 0;
	while ((output.getPositionCount() < nbSamplesMin || output.getPositionCount() > nbSamplesMax) && iter < maxIterations) {
		iter++;
		// let's refine
		if (output.getPositionCount() > nbSamplesMin) { // need to remove some points
			minBound = value;
			if (maxBound == 0) {
				value = minBound * 2;
			}
			else {
				value = minBound + (maxBound - minBound) / 2;
			}
		}
		if (output.getPositionCount() < nbSamplesMax) { // need to add some points
			maxBound = value;
			value = minBound + (maxBound - minBound) / 2;
		}
		std::cout << "  posCount=" << output.getPositionCount() << std::endl;
		std::cout << "  minBound=" << minBound << std::endl;
		std::cout << "  maxBound=" << maxBound << std::endl;
		std::cout << "  value=" << value << std::endl;
		//
		output.reset();
		meshToPcDiv(input, output, tex_map, value, 0, bilinear, logProgress);
	}

	computedThres = value;

	std::cout << "algorithm ended after " << iter << " iterations " << std::endl;
}

//
//          v2
//   		/\
//         /  \
//     e1 /----\ e2
//       / \  / \ 
//      /   \/   \
//    v1 -------- v3
//          e3
//
void subdivideTriangleEdge(
	const Vertex& v1,
	const Vertex& v2,
	const Vertex& v3,
	const Image& tex_map,
	const float lengthThreshold,
	const bool bilinear,
	ModelBuilder& output
) {

	// the face normal
	glm::vec3 normal;
	triangleNormal(v1.pos, v2.pos, v3.pos, normal);

	// do we split the edges: length(edge)/2 >= threshold
	const bool split1 = glm::length(v2.pos - v1.pos) * 0.5F >= lengthThreshold;
	const bool split2 = glm::length(v2.pos - v3.pos) * 0.5F >= lengthThreshold;
	const bool split3 = glm::length(v3.pos - v1.pos) * 0.5F >= lengthThreshold;

	// early return if threshold reached for each edge
	if (!split1 && !split2 && !split3)
		return;

	// the edge centers ~ the potential new vertices
	Vertex e1, e2, e3;
	// we use v1 as reference in term of components to push
	e1.hasColor = e2.hasColor = e3.hasColor = v1.hasColor;
	e1.hasUVCoord = e2.hasUVCoord = e3.hasUVCoord = v1.hasUVCoord;
	// forces normals, we use generated per face ones
	e1.hasNormal = e2.hasNormal = e3.hasNormal = true;

	// compute and push the new edge centers if needed,
	if (split1) {
		e1.pos = (v1.pos + v2.pos) * 0.5F;
		e1.col = (v1.col + v2.col) * 0.5F;
		e1.uv = (v1.uv + v2.uv) * 0.5F;
		e1.nrm = normal;
		output.pushVertex(e1, tex_map, bilinear);
	}
	if (split2) {
		e2.pos = (v2.pos + v3.pos) * 0.5F;
		e2.col = (v2.col + v3.col) * 0.5F;
		e2.uv = (v2.uv + v3.uv) * 0.5F;
		e2.nrm = normal;
		output.pushVertex(e2, tex_map, bilinear);
	}
	if (split3) {
		e3.pos = (v3.pos + v1.pos) * 0.5F;
		e3.col = (v3.col + v1.col) * 0.5F;
		e3.uv = (v3.uv + v1.uv) * 0.5F;
		e3.nrm = normal;
		output.pushVertex(e3, tex_map, bilinear);
	}

	// go deeper in the subdivision if needed
	// three edge split
	if (split1 && split2 && split3) {
		subdivideTriangleEdge(e1, e2, e3, tex_map, lengthThreshold, bilinear, output);
		subdivideTriangleEdge(v1, e1, e3, tex_map, lengthThreshold, bilinear, output);
		subdivideTriangleEdge(e1, v2, e2, tex_map, lengthThreshold, bilinear, output);
		subdivideTriangleEdge(e2, v3, e3, tex_map, lengthThreshold, bilinear, output);
		return;
	}
	// two edge split
	if (!split1 && split2 && split3) {
		subdivideTriangleEdge(v1, v2, e3, tex_map, lengthThreshold, bilinear, output);
		subdivideTriangleEdge(e3, v2, e2, tex_map, lengthThreshold, bilinear, output);
		subdivideTriangleEdge(e2, v3, e3, tex_map, lengthThreshold, bilinear, output);
		return;
	}
	if (split1 && !split2 && split3) {
		subdivideTriangleEdge(v1, e1, e3, tex_map, lengthThreshold, bilinear, output);
		subdivideTriangleEdge(e1, v2, e3, tex_map, lengthThreshold, bilinear, output);
		subdivideTriangleEdge(v2, v3, e3, tex_map, lengthThreshold, bilinear, output);
		return;
	}
	if (split1 && split2 && !split3) {
		subdivideTriangleEdge(v1, e1, v3, tex_map, lengthThreshold, bilinear, output);
		subdivideTriangleEdge(e1, e2, v3, tex_map, lengthThreshold, bilinear, output);
		subdivideTriangleEdge(e1, v2, e2, tex_map, lengthThreshold, bilinear, output);
		return;
	}
	// one edge split
	if (!split1 && !split2 && split3) {
		subdivideTriangleEdge(v1, v2, e3, tex_map, lengthThreshold, bilinear, output);
		subdivideTriangleEdge(v2, v3, e3, tex_map, lengthThreshold, bilinear, output);
		return;
	}
	if (!split1 && split2 && !split3) {
		subdivideTriangleEdge(v1, v2, e2, tex_map, lengthThreshold, bilinear, output);
		subdivideTriangleEdge(v1, e2, v3, tex_map, lengthThreshold, bilinear, output);
		return;
	}
	if (split1 && !split2 && !split3) {
		subdivideTriangleEdge(v1, e1, v3, tex_map, lengthThreshold, bilinear, output);
		subdivideTriangleEdge(e1, v2, v3, tex_map, lengthThreshold, bilinear, output);
		return;
	}
}

// Use triangle subdivision algorithm to perform the sampling
// Use subdiv scheme without T-vertices, stop criterion on edge size
void Sample::meshToPcDivEdge(
	const Model& input, Model& output,
	const Image& tex_map, float lengthThreshold, size_t resolution,
	bool bilinear, bool logProgress, float& computedThres)
{
	float length = lengthThreshold;

	if (length == 0) {
		std::cout << "Overriding lengthThreshold from resolution=" << resolution << std::endl;
		if (resolution == 0) {
			std::cout << "Error: resolution must be > 0 if lengthThreshold = 0.0" << std::endl;
			return;
		}
		// computes the bounding box of the vertices
		glm::vec3 minPos, maxPos;
		computeBBox(input.vertices, minPos, maxPos);
		std::cout << "  minbox = " << minPos[0] << "," << minPos[1] << "," << minPos[2] << std::endl;
		std::cout << "  maxbox = " << maxPos[0] << "," << maxPos[1] << "," << maxPos[2] << std::endl;
		// computes the edge length threshold from resolution of the largest size of the bbox
		const glm::vec3 diag = maxPos - minPos;
		const float boxMaxSize = std::max(diag.x, std::max(diag.y, diag.z));
		length = boxMaxSize / resolution;
		std::cout << "  lengthThreshold = " << length << std::endl;
		computedThres = length; // forwards to caller, only if internally computed
	}

	// to prevent storing duplicate points, we use a ModelBuilder
	ModelBuilder builder(output);

	// number of degenerate triangles
	size_t skipped = 0;

	// For each triangle
	for (size_t triIdx = 0; triIdx < input.triangles.size() / 3; ++triIdx) {

		if (logProgress)
			std::cout << '\r' << triIdx << "/" << input.triangles.size() / 3 << std::flush;

		Vertex v1, v2, v3;

		fetchTriangle(input, triIdx,
			input.uvcoords.size() != 0,
			input.colors.size() != 0,
			input.normals.size() != 0,
			v1, v2, v3);

		// check if triangle is not degenerate
		if (triangleArea(v1.pos, v2.pos, v3.pos) < DBL_EPSILON) {
			++skipped;
			continue;
		}

		// compute face normal (forces) - might be better as an option
		glm::vec3 normal;
		triangleNormal(v1.pos, v2.pos, v3.pos, normal);
		v1.nrm = v2.nrm = v3.nrm = normal;
		v1.hasNormal = v2.hasNormal = v3.hasNormal = true;

		// push the vertices if needed
		builder.pushVertex(v1, tex_map, bilinear);
		builder.pushVertex(v2, tex_map, bilinear);
		builder.pushVertex(v3, tex_map, bilinear);

		// subdivide recursively
		subdivideTriangleEdge(v1, v2, v3, tex_map, length, bilinear, builder);

	}
	if (logProgress)
		std::cout << std::endl;
	if (skipped != 0)
		std::cout << "Skipped " << skipped << " degenerate triangles" << std::endl;
	if (builder.foundCount != 0)
		std::cout << "Handled " << builder.foundCount << " duplicate vertices" << std::endl;
	std::cout << "Generated " << output.vertices.size() / 3 << " points" << std::endl;

}

void Sample::meshToPcDivEdge(
	const Model& input, Model& output, const Image& tex_map,
	size_t nbSamplesMin, size_t nbSamplesMax, size_t maxIterations,
	bool bilinear, bool logProgress, float& computedThres)
{
	float value = 1.0;
	float unused;
	meshToPcDivEdge(input, output, tex_map, value, 0, bilinear, logProgress, unused);
	// search to init the algo bounds
	float minBound = 0;
	float maxBound = 0;
	//
	size_t iter = 0;
	while ((output.getPositionCount() < nbSamplesMin || output.getPositionCount() > nbSamplesMax) && iter < maxIterations) {
		iter++;
		// let's refine
		if (output.getPositionCount() > nbSamplesMin) { // need to remove some points
			minBound = value;
			if (maxBound == 0) {
				value = minBound * 2;
			}
			else {
				value = minBound + (maxBound - minBound) / 2;
			}
		}
		if (output.getPositionCount() < nbSamplesMax) { // need to add some points
			maxBound = value;
			value = minBound + (maxBound - minBound) / 2;
		}
		std::cout << "  posCount=" << output.getPositionCount() << std::endl;
		std::cout << "  minBound=" << minBound << std::endl;
		std::cout << "  maxBound=" << maxBound << std::endl;
		std::cout << "  value=" << value << std::endl;
		//
		output.reset();
		meshToPcDivEdge(input, output, tex_map, value, 0, bilinear, logProgress, unused);
	}
	computedThres = value;
	std::cout << "algorithm ended after " << iter << " iterations " << std::endl;
}

// Use uniform sampling algorithm
// Stop criterion generated point count
// Implementation of the algorithm described in
// http://extremelearning.com.au/evenly-distributing-points-in-a-triangle/

void Sample::meshToPcPrnd(
	const Model& input, Model& output, const Image& tex_map,
	size_t targetPointCount, bool bilinear, bool logProgress)
{
	// number of degenerate triangles
	size_t skipped = 0;

	// to prevent storing duplicate points, we use a ModelBuilder
	ModelBuilder builder(output);
	double totalArea = 0.0f;
	for (size_t triIdx = 0; triIdx < input.triangles.size() / 3; ++triIdx) {
		Vertex v1, v2, v3;

		fetchTriangle(input, triIdx,
			input.uvcoords.size() != 0,
			input.colors.size() != 0,
			input.normals.size() != 0,
			v1, v2, v3);
		totalArea += triangleArea(v1.pos, v2.pos, v3.pos);
	}

	const auto g = 1.0f / 1.32471795572f;
	const auto g2 = g * g;

	// For each triangle
	for (size_t triIdx = 0; triIdx < input.triangles.size() / 3; ++triIdx) {

		if (logProgress)
			std::cout << '\r' << triIdx << "/" << input.triangles.size() / 3 << std::flush;

		Vertex v1, v2, v3;
		fetchTriangle(input, triIdx,
			input.uvcoords.size() != 0,
			input.colors.size() != 0,
			input.normals.size() != 0,
			v1, v2, v3);


		// check if triangle is not degenerate
		if (triangleArea(v1.pos, v2.pos, v3.pos) < DBL_EPSILON) {
			++skipped;
			continue;
		}

		const auto triArea = triangleArea(v1.pos, v2.pos, v3.pos);
		const auto pointCount = std::ceil(targetPointCount * triArea / totalArea);

		// compute face normal (forces) - might be better as an option
		glm::vec3 normal;
		triangleNormal(v1.pos, v2.pos, v3.pos, normal);
		v1.nrm = v2.nrm = v3.nrm = normal;
		v1.hasNormal = v2.hasNormal = v3.hasNormal = true;

		// push the vertices
		builder.pushVertex(v1, tex_map, bilinear);
		builder.pushVertex(v2, tex_map, bilinear);
		builder.pushVertex(v3, tex_map, bilinear);


		const auto d12 = glm::distance(v1.pos, v2.pos);
		const auto d23 = glm::distance(v2.pos, v3.pos);
		const auto d31 = glm::distance(v3.pos, v1.pos);

		glm::vec3 dpos0, dpos1, pos;
		glm::vec2 duv0, duv1, uv;
		if (d12 >= d23 && d12 >= d31) {
			uv = v3.uv;
			duv0 = v1.uv - v3.uv;
			duv1 = v2.uv - v3.uv;

			pos = v3.pos;
			dpos0 = v1.pos - v3.pos;
			dpos1 = v2.pos - v3.pos;
		}
		else if (d31 >= d23) {
			uv = v2.uv;
			duv0 = v1.uv - v2.uv;
			duv1 = v3.uv - v2.uv;

			pos = v2.pos;
			dpos0 = v1.pos - v2.pos;
			dpos1 = v3.pos - v2.pos;
		}
		else {
			uv = v1.uv;
			duv0 = v2.uv - v1.uv;
			duv1 = v3.uv - v1.uv;

			pos = v1.pos;
			dpos0 = v2.pos - v1.pos;
			dpos1 = v3.pos - v1.pos;
		}

		// the new vertices
		Vertex vertex;
		// we use v1 as reference in term of components to push
		vertex.hasColor = v1.hasColor;
		vertex.hasUVCoord = v1.hasUVCoord;
		// forces normals, we use generated per face ones 
		vertex.hasNormal = true;

		for (int i = 1; i < pointCount; ++i) {
			const auto r1 = i * g;
			const auto r2 = i * g2;
			auto x = r1 - std::floor(r1);
			auto y = r2 - std::floor(r2);
			if (x + y > 1.0f) {
				x = 1.0f - x;
				y = 1.0f - y;
			}
			vertex.pos = pos + x * dpos0 + y * dpos1;
			vertex.uv = uv + x * duv0 + y * duv1;
			vertex.nrm = normal;
			builder.pushVertex(vertex, tex_map, bilinear);
		}
	}
	if (logProgress)
		std::cout << std::endl;
	if (skipped != 0)
		std::cout << "Skipped " << skipped << " degenerate triangles" << std::endl;
	if (builder.foundCount != 0)
		std::cout << "Handled " << builder.foundCount << " duplicate vertices" << std::endl;
	std::cout << "Generated " << output.vertices.size() / 3 << " points" << std::endl;
}

