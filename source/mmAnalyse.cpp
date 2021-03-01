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
#include "mmAnalyse.h"
#include "mmIO.h"
#include "mmModel.h"
#include "mmImage.h"
#include "mmGeometry.h"
#include "mmStatistics.h"

const char* Analyse::name = "analyse";
const char* Analyse::brief = "Analyse model and/or texture map";

// 
Command* Analyse::create() { return new Analyse(); }

//
bool Analyse::initialize(Context* context, std::string app, int argc, char* argv[])
{
	_context = context;

	// command line parameters
	try
	{
		cxxopts::Options options(app + " " + name, brief);
		options.add_options()
			("inputModel", "path to input model (obj or ply file)",
				cxxopts::value<std::string>())
			("inputMap", "path to input texture map (png, jpeg)",
				cxxopts::value<std::string>())
			("outputCsv", "optional path to output results file",
				cxxopts::value<std::string>())
			("outputVar", "optional path to output variables file",
				cxxopts::value<std::string>())
			("h,help", "Print usage")
			;

		auto result = options.parse(argc, argv);

		// Analyse the options
		if (result.count("help") || result.arguments().size() == 0)
		{
			std::cout << options.help() << std::endl;
			return false;
		}
		// Optional
		if (result.count("inputModel"))
			_inputModelFilename = result["inputModel"].as<std::string>();
		// Optional
		if (result.count("inputMap"))
			_inputTextureFilename = result["inputMap"].as<std::string>();
		// Optional
		if (result.count("outputCsv"))
			_outputCsvFilename = result["outputCsv"].as<std::string>();
		// Optional
		if (result.count("outputVar"))
			_outputVarFilename = result["outputVar"].as<std::string>();
	}
	catch (const cxxopts::OptionException& e)
	{
		std::cout << "error parsing options: " << e.what() << std::endl;
		return false;
	}

	return true;
}

bool Analyse::process(uint32_t frame) {

	// Reading map if needed
	Image* textureMap=NULL;
	if (_inputTextureFilename != "") {
		textureMap = IO::loadImage(_inputTextureFilename);
	}

	// the input
	Model* inputModel=NULL;
	if (_inputModelFilename != "") {
		if ((inputModel = IO::loadModel(_inputModelFilename)) == NULL) {
			return false;
		}
	}

	// create or open in append mode output csv if needed
	std::ofstream fout;
	if (_outputCsvFilename != ""){
		if (frame == _context->getFirstFrame()) {
			fout.open(_outputCsvFilename.c_str(), std::ios::out);
		}
		else {
			fout.open(_outputCsvFilename.c_str(), std::ios::out | std::ofstream::app );
		}
		// this is mandatory to print floats with full precision
		fout.precision(std::numeric_limits< float >::max_digits10);
	}

	// Perform the processings
	clock_t t1 = clock();

	glm::vec3 minPos, maxPos;
	glm::vec3 minNrm, maxNrm;
	glm::vec3 minCol, maxCol;
	glm::vec2 minUv, maxUv;

	// analyse the model if any
	if (inputModel != NULL) {
		
		_counts.push_back(std::make_tuple(_context->getFrame(), 
			(double)inputModel->triangles.size() / 3,
			(double)inputModel->vertices.size() / 3,
			(double)inputModel->colors.size() / 3,
			(double)inputModel->normals.size() / 3,
			(double)inputModel->uvcoords.size() / 3
		));
		
		computeBBox(inputModel->vertices, minPos, maxPos);
		computeBBox(_minPos, _maxPos, minPos, maxPos, _minPos, _maxPos);
		if (inputModel->normals.size()){
			computeBBox(inputModel->normals, minNrm, maxNrm);
			computeBBox(_minNrm, _maxNrm, minNrm, maxNrm, _minNrm, _maxNrm);
		}
		if (inputModel->colors.size()) {
			computeBBox(inputModel->colors, minCol, maxCol);
			computeBBox(_minCol, _maxCol, minCol, maxCol, _minCol, _maxCol);
		}
		if (inputModel->uvcoords.size()) {
			computeBBox(inputModel->uvcoords, minUv, maxUv);
			computeBBox(_minUv, _maxUv, minUv, maxUv, _minUv, _maxUv);
		}
		
		// TODO: add more stats
		// find degenerate triangles ?
		// count isolated vertices ?
	}

	// analyse the texture if any
	if (textureMap != NULL) {

	}

	// print to output csv if needed
	if (fout) {
		// print the header if needed
		if (frame == _context->getFirstFrame()) {
			fout << "frame";
			if (inputModel != NULL) {
				fout << ";triangles;vertices;uvcoords;colors;normals"
					 << ";minPosX;minPosY;minPosZ;maxPosX;maxPosY;maxPosZ"
					 << ";minU;minV;maxU;maxV"
					 << ";minColR;minColG;minColB;maxColR;maxColB;maxColB"
					 << ";minNrmY;minNrmY;minNrmZ;maxNrmY;maxNrmY;maxNrmZ";
			}
			if (textureMap != NULL) {
				fout << ""; // nothing yet
			}
			fout << std::endl;
		}
		// print stats
		fout << frame ;
		if (inputModel != NULL) {
			fout << ";" << inputModel->triangles.size() / 3 \
				<< ";" << inputModel->vertices.size() / 3 \
				<< ";" << inputModel->normals.size() / 3 \
				<< ";" << inputModel->colors.size() / 3 \
				<< ";" << inputModel->uvcoords.size() / 2 \
				<< ";" << minPos[0] << ";" << minPos[1] << ";" << minPos[2] \
				<< ";" << maxPos[0] << ";" << maxPos[1] << ";" << maxPos[2];
			if (inputModel->uvcoords.size()) {
				fout << ";" << minUv[0] << ";" << minUv[1] \
					<< ";" << maxUv[0] << ";" << maxUv[1];
			}
			else fout << ";;;;";
			if (inputModel->colors.size()) {
				fout << ";" << minCol[0] << ";" << minCol[1] << ";" << minCol[2] \
					<< ";" << maxCol[0] << ";" << maxCol[1] << ";" << maxCol[2];
			}
			else fout << ";;;;;;";
			if (inputModel->normals.size()) {
				fout << ";" << minNrm[0] << ";" << minNrm[1] << ";" << minNrm[2] \
					<< ";" << maxNrm[0] << ";" << maxNrm[1] << ";" << maxNrm[2];
			}
			else fout << ";;;;;;";
		}
		fout << std::endl;
		// done
		fout.close();
	}

	// done
	clock_t t2 = clock();
	std::cout << "Time on processing: " << ((float)(t2 - t1)) / CLOCKS_PER_SEC << " sec." << std::endl;

	// success
	return true;
}

bool Analyse::finalize() {

	std::vector<std::ostream*> out;
	out.push_back(&std::cout);
	std::ofstream fout;
	if (_outputVarFilename != "") {
		fout.open(_outputVarFilename.c_str(), std::ios::out);
		if ( fout ){
			out.push_back(&fout);
		}
		else {
			std::cout << "Error: could not create output file " << _outputVarFilename << std::endl;
		}
	}

	for (size_t i = 0; i < out.size(); ++i) {

		// this is mandatory to print floats with full precision
		out[i]->precision(std::numeric_limits< float >::max_digits10);

		// display the statistics (compatioble with bash source)
		*out[i] << "firstFrame=" << _context->getFirstFrame() << std::endl;
		*out[i] << "lastFrame=" << _context->getLastFrame() << std::endl;

		Statistics::Results stats;
		Statistics::compute(_counts.size(), [&](size_t i) -> double { return std::get<1>(_counts[i]); }, stats);
		Statistics::printToLog(stats, "globalTriangleCount", *out[i] );
		Statistics::compute(_counts.size(), [&](size_t i) -> double { return std::get<2>(_counts[i]); }, stats);
		Statistics::printToLog(stats, "globalVertexCount", *out[i] );
		Statistics::compute(_counts.size(), [&](size_t i) -> double { return std::get<3>(_counts[i]); }, stats);
		Statistics::printToLog(stats, "globalColorCount", *out[i] );
		Statistics::compute(_counts.size(), [&](size_t i) -> double { return std::get<4>(_counts[i]); }, stats);
		Statistics::printToLog(stats, "globalNormalCount", *out[i]);
		Statistics::compute(_counts.size(), [&](size_t i) -> double { return std::get<5>(_counts[i]); }, stats);
		Statistics::printToLog(stats, "globalUvCoordCount", *out[i]);

		*out[i] << "globalMinPos=\"" << _minPos[0] << " " << _minPos[1] << " " << _minPos[2] << "\"" << std::endl;
		*out[i] << "globalMaxPos=\"" << _maxPos[0] << " " << _maxPos[1] << " " << _maxPos[2] << "\"" << std::endl;

		*out[i] << "globalMinUv=\"" << _minUv[0] << " " << _minUv[1] << "\"" << std::endl;
		*out[i] << "globalMaxUv=\"" << _maxUv[0] << " " << _maxUv[1] << "\"" << std::endl;

		*out[i] << "globalMinNrm=\"" << _minNrm[0] << " " << _minNrm[1] << " " << _minNrm[2] << "\"" << std::endl;
		*out[i] << "globalMaxNrm=\"" << _maxNrm[0] << " " << _maxNrm[1] << " " << _maxNrm[2] << "\"" << std::endl;

		*out[i] << "globalMinCol=\"" << _minCol[0] << " " << _minCol[1] << " " << _minCol[2] << "\"" << std::endl;
		*out[i] << "globalMaxCol=\"" << _maxCol[0] << " " << _maxCol[1] << " " << _maxCol[2] << "\"" << std::endl;
	}
	// done
	if ( fout ) fout.close();

	return true;
}
