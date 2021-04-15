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

// 
#include "pcc/pcc_processing.hpp"
#include "pcqm/pcqm.h"
// internal headers
#include "mmIO.h"
#include "mmModel.h"
#include "mmImage.h"
#include "mmCompare.h"
#include "mmSample.h"
#include "mmGeometry.h"
#include "mmStatistics.h"

const char* Compare::name = "compare";
const char* Compare::brief = "Compare model A vs model B";

// 
Command* Compare::create() { return new Compare(); }

//
bool Compare::initialize(Context* context, std::string app, int argc, char* argv[])
{
	_context = context;

	// command line parameters
	try
	{
		cxxopts::Options options(app + " " + name, brief);
		options.add_options()
			("inputModelA", "path to reference input model (obj or ply file)",
				cxxopts::value<std::string>())
			("inputModelB", "path to distorted input model (obj or ply file)",
				cxxopts::value<std::string>())
			("inputMapA", "path to reference input texture map (png, jpg, rgb, yuv)",
				cxxopts::value<std::string>())
			("inputMapB", "path to distorted input texture map (png, jpg, rgb, yuv)",
				cxxopts::value<std::string>())
			("outputModelA", "path to output model A (obj or ply file)",
				cxxopts::value<std::string>())
			("outputModelB", "path to output model B (obj or ply file)",
				cxxopts::value<std::string>())
			("mode", "the comparison mode in [equ,pcc,pcqm]",
				cxxopts::value<std::string>()->default_value("equ"))
			("h,help", "Print usage")
			;
		options.add_options("equ mode")
			("epsilon", "Used for point cloud comparison only. Distance threshold in world units for \"equality\" comparison. If 0.0 use strict equality (no distace computation).",
				cxxopts::value<float>()->default_value("0.0"))
			("earlyReturn", "Return as soon as a difference is found (faster). Otherwise provide more complete report (slower).",
				cxxopts::value<bool>()->default_value("true"))
			("unoriented", "If set, comparison will not consider faces orientation for comparisons.",
				cxxopts::value<bool>()->default_value("false"))
			;
		options.add_options("pcc mode")
			("singlePass", "Force running a single pass, where the loop is over the original point cloud",
				cxxopts::value<bool>()->default_value("false"))
			("hausdorff", "Send the Haursdorff metric as well",
				cxxopts::value<bool>()->default_value("false"))
			("color", "Check color distortion as well",
				cxxopts::value<bool>()->default_value("true"))
			("resolution", "Amplitude of the geometric signal. Will be automatically set to diagonal of the models bounding box if value = 0",
				cxxopts::value<float>()->default_value("0.0"))
			("neighborsProc", "0(undefined), 1(average), 2(weighted average) 3(min), 4(max) neighbors with same geometric distance",
				cxxopts::value<int>()->default_value("1"))
			("dropDuplicates", "0(detect), 1(drop), 2(average) subsequent points with same coordinates",
				cxxopts::value<int>()->default_value("2"))
			("bAverageNormals", "false(use provided normals), true(average normal based on neighbors with same geometric distance)",
				cxxopts::value<bool>()->default_value("true"))
			;
		options.add_options("pcqm mode")
			("radiusCurvature", "Set a radius for the construction of the neighborhood. As the bounding box is already computed with this program, use proposed value.",
				cxxopts::value<double>()->default_value("0.001"))
			("thresholdKnnSearch", "Set the number of points used for the quadric surface construction",
				cxxopts::value<int>()->default_value("20"))
			("radiusFactor", "Set a radius factor for the statistic computation.",
				cxxopts::value<double>()->default_value("2.0"))

			;
		options.add_options("pcc and pcqm modes, near lossless")
			("topologyFile", "path to the topology text file matching modelB topology to modelA topology. Setting option with non empty filename will activate the test.",
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
		if (result.count("inputModelA"))
			inputModelAFilename = result["inputModelA"].as<std::string>();
		else {
			std::cerr << "Error: missing inputModelA parameter" << std::endl;
			std::cout << options.help() << std::endl;
			return false;
		}
		//	
		if (result.count("inputModelB"))
			inputModelBFilename = result["inputModelB"].as<std::string>();
		else {
			std::cerr << "Error: missing inputModelB parameter" << std::endl;
			std::cout << options.help() << std::endl;
			return false;
		}
		// Optional
		if (result.count("inputMapA"))
			inputTextureAFilename = result["inputMapA"].as<std::string>();
		// Optional
		if (result.count("inputMapB"))
			inputTextureBFilename = result["inputMapB"].as<std::string>();
		// Optional
		if (result.count("outputModelA"))
			outputModelAFilename = result["outputModelA"].as<std::string>();
		if (result.count("outputModelB"))
			outputModelBFilename = result["outputModelB"].as<std::string>();
		//
		if (result.count("mode"))
			mode = result["mode"].as<std::string>();
		// eq
		if (result.count("epsilon"))
			epsilon = result["epsilon"].as<float>();
		if (result.count("earlyReturn"))
			earlyReturn = result["earlyReturn"].as<bool>();
		if (result.count("unoriented"))
			unoriented = result["unoriented"].as<bool>();
		// PCC
		if (result.count("singlePass"))
			params.singlePass = result["singlePass"].as<bool>();
		if (result.count("hausdorff"))
			params.hausdorff = result["hausdorff"].as<bool>();
		if (result.count("color"))
			params.bColor = result["color"].as<bool>();
		if (result.count("resolution"))
			params.resolution = result["resolution"].as<float>();
		if (result.count("dropDuplicates"))
			params.dropDuplicates = result["dropDuplicates"].as<int>();
		if (result.count("neighborsProc"))
			params.neighborsProc = result["neighborsProc"].as<int>();
		if (result.count("averageNormals"))
			params.bAverageNormals = result["averageNormals"].as<bool>();
		// PCQM
		if (result.count("radiusCurvature"))
			radiusCurvature = result["radiusCurvature"].as<double>();
		if (result.count("thresholdKnnSearch"))
			thresholdKnnSearch = result["thresholdKnnSearch"].as<int>();
		if (result.count("radiusFactor"))
			radiusFactor = result["radiusFactor"].as<double>();
		// common to PCC and PCQM
		if (result.count("topologyFile"))
			topologyFilename = result["topologyFile"].as<string>();

	}
	catch (const cxxopts::OptionException& e)
	{
		std::cout << "error parsing options: " << e.what() << std::endl;
		return false;
	}

	return true;
}

bool Compare::process(uint32_t frame) {

	// Reading map if needed
	Image* textureMapA, * textureMapB;
	bool perVertexColor = false;
	if (inputTextureAFilename != "") {
		textureMapA = IO::loadImage(inputTextureAFilename);
	}
	else {
		std::cout << "Skipping map read, will parse use vertex color if any" << std::endl;
		textureMapA = new Image();
		perVertexColor = true;
	}
	if (inputTextureBFilename != "") {
		textureMapB = IO::loadImage(inputTextureBFilename);
		if (perVertexColor) {
			std::cout << "Error: inputs model colors are not homogeneous " << std::endl;
			return false;
		}
	}
	else {
		std::cout << "Skipping map read, will parse use vertex color if any" << std::endl;
		textureMapB = new Image();
		perVertexColor = true;
	}

	// the input
	Model* inputModelA, * inputModelB;
	if ((inputModelA = IO::loadModel(inputModelAFilename)) == NULL) {
		return false;
	}
	if (inputModelA->vertices.size() == 0) {
		std::cout << "Error: input model from " << inputModelAFilename << " has no vertices" << std::endl;
		return false;
	}
	if ((inputModelB = IO::loadModel(inputModelBFilename)) == NULL) {
		return false;
	}
	if (inputModelB->vertices.size() == 0) {
		std::cout << "Error: input model from " << inputModelBFilename << " has no vertices" << std::endl;
		return false;
	}

	// the output models if any
	Model* outputModelA = new Model();
	Model* outputModelB = new Model();
	int res = 2;

	// Perform the processings
	clock_t t1 = clock();
	if (mode == "equ") {
		std::cout << "Compare models for equality" << std::endl;
		std::cout << "  Epsilon = " << epsilon << std::endl;
		res = Compare::equ(
			*inputModelA, *inputModelB,
			*textureMapA, *textureMapB,
			epsilon, earlyReturn, unoriented,
			*outputModelA, *outputModelB);
	}
	else if (mode == "pcc") {
		std::cout << "Compare models using MPEG PCC distortion metric" << std::endl;
		std::cout << "  topologyFile = " << topologyFilename << std::endl;
		std::cout << "  singlePass = " << params.singlePass << std::endl;
		std::cout << "  hausdorff = " << params.hausdorff << std::endl;
		std::cout << "  color = " << params.bColor << std::endl;
		std::cout << "  resolution = " << params.resolution << std::endl;
		std::cout << "  neighborsProc = " << params.neighborsProc << std::endl;
		std::cout << "  dropDuplicates = " << params.dropDuplicates << std::endl;
		std::cout << "  AverageNormals = " << params.bAverageNormals << std::endl;
		res = Compare::pcc(
			*inputModelA, *inputModelB,
			*textureMapA, *textureMapB, params,
			*outputModelA, *outputModelB, topologyFilename);
	}
	else if (mode == "pcqm") {
		std::cout << "Compare models using PCQM distortion metric" << std::endl;
		std::cout << "  topologyFile = " << topologyFilename << std::endl;
		std::cout << "  radiusCurvature = " << radiusCurvature << std::endl;
		std::cout << "  thresholdKnnSearch = " << thresholdKnnSearch << std::endl;
		std::cout << "  radiusFactor = " << radiusFactor << std::endl;
		res = Compare::pcqm(
			*inputModelA, *inputModelB,
			*textureMapA, *textureMapB,
			radiusCurvature,
			thresholdKnnSearch,
			radiusFactor,
			*outputModelA, *outputModelB, topologyFilename);
	}
	else {
		std::cerr << "Error: invalid --mode " << mode << std::endl;
		return false;
	}
	clock_t t2 = clock();
	std::cout << "Time on processing: " << ((float)(t2 - t1)) / CLOCKS_PER_SEC << " sec." << std::endl;

	// save the result
	if (outputModelAFilename != "") {
		if (!IO::saveModel(outputModelAFilename, outputModelA))
			return false;
	}
	// save the result
	if (outputModelBFilename != "") {
		if (!IO::saveModel(outputModelBFilename, outputModelB))
			return false;
	}
	// success
	std::cout << "return " << res << std::endl;
	return true;
}

bool Compare::finalize() {

	// Collect the statistics
	if (mode == "equ") {
		// nothing to do
	}
	else if (mode == "pcc") {
		pccFinalize();
	}
	else if (mode == "pcqm") {
		pcqmFinalize();
	}
	else {
		std::cerr << "Error: invalid --mode " << mode << std::endl;
		return false;
	}

	return true;
}

// A 1 2 3
// B 1 2 3
//    / /
//   2 3 1 
//    / /
//   3 1 2 
// twosided (we turn in other direction)
// B 1 3 2
//    / /
//   3 2 1 
//    / /
//   2 1 3 

bool areTrianglesEqual(bool unoriented,
	Vertex& vA1, Vertex& vA2, Vertex& vA3,
	Vertex& vB1, Vertex& vB2, Vertex& vB3) 
{
	return 
		(vA1 == vB1 && vA2 == vB2 && vA3 == vB3) ||
		(vA1 == vB2 && vA2 == vB3 && vA3 == vB1) ||
		(vA1 == vB3 && vA2 == vB1 && vA3 == vB2) ||
		(unoriented && (
			(vA1 == vB1 && vA2 == vB3 && vA3 == vB2) ||
			(vA1 == vB3 && vA2 == vB2 && vA3 == vB1) ||
			(vA1 == vB2 && vA2 == vB1 && vA3 == vB3)
		));
}

int Compare::equ(
	const Model& inputA, const Model& inputB,
	const Image& mapA, const Image& mapB,
	float epsilon, bool earlyReturn, bool unoriented,
	Model& outputA, Model& outputB)
{	

	// we test the maps
	if (mapA.data != NULL || mapB.data != NULL){
		if (mapA.data == NULL){
			std::cout << "texture maps are not equal: mapA is null" << std::endl;
		}
		else if (mapB.data == NULL) {
			std::cout << "texture maps are not equal: mapB is null" << std::endl;
		}
		else {
			if (mapA.width != mapB.width || mapA.height != mapB.height) {
				std::cout << "texture maps are not equal: dimensions are not equal" << std::endl;
			}
			else {
				size_t diffs = 0;
				for (size_t row = 0; row < mapA.height; ++row) {
					for (size_t col = 0; col < mapA.width; ++col) {
						glm::vec3 colA, colB;
						mapA.fetchRGB(col, row, colA);
						mapB.fetchRGB(col, row, colB);
						if (colA != colB)
							++diffs;
					}
				}
				if (diffs != 0) {
					std::cout << "texture maps are not equal: " << diffs << "pixel differences" << std::endl;
				}
				else {
					std::cout << "texture maps are equal" << std::endl;
				}
			}
		}
	}
	else {
		std::cout << "skipping texture maps comparison" << std::endl;
	}

	if (inputA.triangles.size() != inputB.triangles.size()) {
		std::cout << "meshes are not equal, number of triangles are different " << 
			inputA.triangles.size() / 3 << " vs " << inputB.triangles.size() / 3 << std::endl;
		return true;
	}

	// mesh mode
	if (inputA.triangles.size() != 0) {

		// prepare a store for face status 
		// doundInB[true] if the nth face of B matches one face of A
		std::vector<bool> foundInB(inputB.getTriangleCount(), false);

		const bool hasColors = inputA.hasColors() && inputB.hasColors();
		const bool hasUvCoords = inputA.hasUvCoords() && inputB.hasUvCoords();
		const bool hasNormals = inputA.hasNormals() && inputB.hasNormals();
		
		size_t diffs = 0; // count the differences if no earlyReturn option

		// iterate over triangles of modelA
		for (size_t triIdx = 0; triIdx < inputA.triangles.size() / 3; triIdx++) {
			Vertex v1, v2, v3;
			fetchTriangle(inputA, triIdx, hasUvCoords, hasColors, hasNormals, v1, v2, v3);
			// search over modelB riangles that are not already matched
			bool found = false;
			size_t triIdxB = 0;
			while (!found && triIdxB != inputB.triangles.size() / 3) {
				if (foundInB[triIdxB]) { ++triIdxB; continue; }
				Vertex vB1, vB2, vB3;
				fetchTriangle(inputB, triIdxB, hasUvCoords, hasColors, hasNormals, vB1, vB2, vB3);
				if (areTrianglesEqual(unoriented, v1, v2, v3, vB1, vB2, vB3)) {
					found = true;
					foundInB[triIdxB] = true;
				}
				++triIdxB;
			}
			if (!found) {
				if (earlyReturn) {
					std::cout << "meshes are not equal, early return." << std::endl;
					std::cout << "triangle number " << triIdx  << " from A has no equivalent in B" << std::endl;
					return true;
				}
				++diffs;
			}
		}

		if (diffs == 0) {
			std::cout << "meshes are equal" << std::endl;
		}
		else {
			std::cout << "meshes are not equal, " << diffs << " different triangles" << std::endl;
			// provide more details on what is different at additional computation cost (request from Sony team)
			size_t vertDiffs = std::abs( (long long)(inputA.getPositionCount() - inputB.getPositionCount()) );
			for (size_t index = 0; index < std::min(inputA.getPositionCount(), inputB.getPositionCount()); ++index) {
				if (inputA.fetchPosition(index) != inputB.fetchPosition(index))
					++vertDiffs;
			}
			std::cout << "Position differences: " << vertDiffs << std::endl;
			//
			size_t uvDiffs = std::abs((long long)(inputA.getUvCount() - inputB.getUvCount()));
			for (size_t index = 0; index < std::min(inputA.getUvCount(), inputB.getUvCount()); ++index) {
				if (inputA.fetchUv(index) != inputB.fetchUv(index))
					++uvDiffs;
			}
			std::cout << "UV coords differences: " << uvDiffs << std::endl;
			//
			size_t colorDiffs = std::abs((long long)(inputA.getColorCount() - inputB.getColorCount()));
			for (size_t index = 0; index < std::min(inputA.getColorCount(), inputB.getColorCount()); ++index) {
				if (inputA.fetchColor(index) != inputB.fetchColor(index))
					++colorDiffs;
			}
			std::cout << "Colors differences: " << colorDiffs << std::endl;
			//
			size_t normalDiffs = std::abs((long long)(inputA.getNormalCount() - inputB.getNormalCount()));
			for (size_t index = 0; index < std::min(inputA.getNormalCount(), inputB.getNormalCount()); ++index) {
				if (inputA.fetchNormal(index) != inputB.fetchNormal(index))
					++normalDiffs;
			}
			std::cout << "Normals differences: " << normalDiffs << std::endl;
		}
		return true;
	}
	// Point cloud mode, sort vertices then compare
	else {
		// allocate room for the results
		outputA.vertices.resize(inputA.vertices.size());
		outputB.vertices.resize(inputB.vertices.size());

		// prepare outputA
		std::vector<Vertex> positions;
		Vertex vertex;
		for (int i = 0; i < inputA.vertices.size() / 3; i++) {
			vertex.pos = glm::vec3(inputA.vertices[i * 3 + 0], inputA.vertices[i * 3 + 1], inputA.vertices[i * 3 + 2]);
			positions.push_back(vertex);
		}
		std::sort(positions.begin(), positions.end(), CompareVertex<true,false,false,false>() );

		for (int i = 0; i < positions.size(); i++) {
			outputA.vertices[i * 3 + 0] = positions[i].pos.x;
			outputA.vertices[i * 3 + 1] = positions[i].pos.y;
			outputA.vertices[i * 3 + 2] = positions[i].pos.z;
		}

		// prepare outputB
		positions.clear();
		for (int i = 0; i < inputB.vertices.size() / 3; i++) {
			vertex.pos = glm::vec3(inputB.vertices[i * 3 + 0], inputB.vertices[i * 3 + 1], inputB.vertices[i * 3 + 2]);
			positions.push_back(vertex);
		}
		std::sort(positions.begin(), positions.end(), CompareVertex<true, false, false, false>());

		for (int i = 0; i < positions.size(); i++) {
			outputB.vertices[i * 3 + 0] = positions[i].pos.x;
			outputB.vertices[i * 3 + 1] = positions[i].pos.y;
			outputB.vertices[i * 3 + 2] = positions[i].pos.z;
		}

		// now compare the results
		if (epsilon == 0) {
			if (outputB.vertices == outputA.vertices) {
				std::cout << "model vertices are equals" << std::endl;
				return true;
			}
			else {
				std::cout << "model vertices are not equals" << std::endl;
				return true;
			}
		}
		else {
			if (outputA.vertices.size() != outputB.vertices.size()) {
				std::cout << "model vertices are not equals" << std::endl;
				return true;
			}
			size_t count = 0;
			for (size_t i = 0; i < outputA.vertices.size() / 3; i++) {
				glm::vec3 A = glm::make_vec3(&outputA.vertices[i * 3]);
				glm::vec3 B = glm::make_vec3(&outputB.vertices[i * 3]);
				if (glm::length(A - B) >= epsilon) {
					++count;
				}
			}
			if (count == 0) {
				std::cout << "model vertices are equals" << std::endl;
			}
			else {
				std::cout << "model vertices are not equals, found " << count << " differences" << std::endl;
			}
			return true;
		}
	}
}

void sampleIfNeeded(
	const Model& input,
	const Image& map,
	Model& output)
{
	if (input.triangles.size() != 0) {

		// first reorder the model to prevent small variations 
		// when having two similar topologies but not same orders of enumeration
		Model reordered;
		reorder(input, std::string("oriented"), reordered);

		// then use face subdivision without map citerion and area threshold of 2.0
		Sample::meshToPcDiv(reordered, output, map, 2.0, false, true, false);

	}
	else {
		output = input; //  pass through
	}
}

// code  from PCC_error (prevents pcc_error library modification.
int removeDuplicatePoints(PccPointCloud& pc, int dropDuplicates, int neighborsProc) {
	// sort the point cloud
	std::sort(pc.begin(), pc.end());

	// Find runs of identical point positions
	for (auto it_seq = pc.begin(); it_seq != pc.end(); ) {
		it_seq = std::adjacent_find(it_seq, pc.end());
		if (it_seq == pc.end())
			break;

		// accumulators for averaging attribute values
		long cattr[3]{}; // sum colors.
		long lattr{}; // sum lidar.

		// iterate over the duplicate points, accumulating values
		int count = 0;
		auto it = it_seq;
		for (; *it == *it_seq; ++it) {
			count++;
			size_t idx = it.idx;
			if (pc.bRgb) {
				cattr[0] += pc.rgb.c[idx][0];
				cattr[1] += pc.rgb.c[idx][1];
				cattr[2] += pc.rgb.c[idx][2];
			}

			if (pc.bLidar)
				lattr += pc.lidar.reflectance[idx];
		}

		size_t first_idx = it_seq.idx;
		it_seq = it;

		// averaging case only
		if (dropDuplicates != 2)
			continue;

		if (pc.bRgb) {
			pc.rgb.c[first_idx][0] = (unsigned char)(cattr[0] / count);
			pc.rgb.c[first_idx][1] = (unsigned char)(cattr[1] / count);
			pc.rgb.c[first_idx][2] = (unsigned char)(cattr[2] / count);
		}

		if (neighborsProc == 2)
			pc.xyz.nbdup[first_idx] = count;

		if (pc.bLidar)
			pc.lidar.reflectance[first_idx] = (unsigned short)(lattr / count);
	}

	int duplicatesFound = 0;
	if (dropDuplicates != 0) {
		auto last = std::unique(pc.begin(), pc.end());
		duplicatesFound = (int)std::distance(last, pc.end());

		pc.size -= duplicatesFound;
		pc.xyz.p.resize(pc.size);
		pc.xyz.nbdup.resize(pc.size);
		if (pc.bNormal)
			pc.normal.n.resize(pc.size);
		if (pc.bRgb)
			pc.rgb.c.resize(pc.size);
		if (pc.bLidar)
			pc.lidar.reflectance.resize(pc.size);
	}

	if (duplicatesFound > 0)
	{
		switch (dropDuplicates)
		{
		case 0:
			printf("WARNING: %d points with same coordinates found\n",
				duplicatesFound);
			break;
		case 1:
			printf("WARNING: %d points with same coordinates found and dropped\n",
				duplicatesFound);
			break;
		case 2:
			printf("WARNING: %d points with same coordinates found and averaged\n",
				duplicatesFound);
		}
	}

	return 0;
}

// utility func used by compare::pcc
// no sanity check, we assume the model is clean 
// and generated by sampling that allways generate content with color and normals
void convertModel(const Model& inputModel, pcc_quality::commandPar& params, PccPointCloud& outputModel) {
	for (size_t i = 0; i < inputModel.vertices.size() / 3; ++i) {
		// push the positions
		outputModel.xyz.p.push_back(std::array<float, 3>({
			inputModel.vertices[i * 3],
			inputModel.vertices[i * 3 + 1],
			inputModel.vertices[i * 3 + 2] }));
		// push the normals if any
		if (inputModel.normals.size() > i * 3 + 2)
			outputModel.normal.n.push_back(std::array<float, 3>({
				inputModel.normals[i * 3],
				inputModel.normals[i * 3 + 1],
				inputModel.normals[i * 3 + 2] }));
		// push the colors if any
		if (inputModel.colors.size() > i * 3 + 2)
			outputModel.rgb.c.push_back(std::array<unsigned char, 3>({
				(unsigned char)(std::roundf(inputModel.colors[i * 3])),
				(unsigned char)(std::roundf(inputModel.colors[i * 3 + 1])),
				(unsigned char)(std::roundf(inputModel.colors[i * 3 + 2])) }));
	}
	outputModel.size = (long)outputModel.xyz.p.size();
	outputModel.bXyz = outputModel.size >= 1;
	if (inputModel.colors.size() == inputModel.vertices.size())
		outputModel.bRgb = true;
	else
		outputModel.bRgb = false;
	if (inputModel.normals.size() == inputModel.vertices.size())
		outputModel.bNormal = true;
	else
		outputModel.bNormal = false;
	outputModel.bLidar = false;

	outputModel.xyz.nbdup.resize(outputModel.xyz.p.size());
	std::fill(outputModel.xyz.nbdup.begin(), outputModel.xyz.nbdup.end(), 1);
	removeDuplicatePoints(outputModel, params.dropDuplicates, params.neighborsProc);

}

int Compare::pcc(
	const Model& modelA, const Model& modelB,
	const Image& mapA, const Image& mapB,
	pcc_quality::commandPar& params,
	Model& outputA, Model& outputB,
	const std::string& topoMapFilenane
) {
	// 0 - check topology if needed
	if (topoMapFilenane.length() != 0) {
		if(checkTopology(topoMapFilenane, modelA, modelB))
		{
			std::cout<<"Given the mapping in "<<topoMapFilenane<<" the topologies correspond to each other"<<std::endl;
		} else {
			std::cout<<"Given the mapping in "<<topoMapFilenane<<" the topologies DO NOT correspond to each other"<<std::endl;
		}
	}

	// 1 - sample the models if needed
	sampleIfNeeded(modelA, mapA, outputA);
	sampleIfNeeded(modelB, mapB, outputB);

	// 2 - transcode to PCC internal format
	pcc_processing::PccPointCloud inCloud1;
	pcc_processing::PccPointCloud inCloud2;

	convertModel(outputA, params, inCloud1);
	convertModel(outputB, params, inCloud2);

	// we use outputA as reference for signal dynamic if needed
	if (params.resolution == 0) {
		glm::vec3 minBox, maxBox;
		computeBBox(outputA.vertices, minBox, maxBox);
		params.resolution = glm::length(maxBox - minBox);
	}
	//
	params.file1 = "dummy1.ply"; // could be any name !="", just force some pcc inner tests to pass
	params.file2 = "dummy2.ply"; // could be any name !="", just force some pcc inner tests to pass
	// compute plane metric if we have valid normal arrays
	params.c2c_only = !(outputA.normals.size() == outputA.vertices.size() && outputB.normals.size() == outputB.vertices.size());
	// compute color metric if valid color arrays (no support for RGBA colors)
	params.bColor = params.bColor && (outputA.colors.size() == outputA.vertices.size() && outputB.colors.size() == outputB.vertices.size());

	// 3 - compute the metric
	pcc_quality::qMetric qm;
	computeQualityMetric(inCloud1, inCloud1, inCloud2, params, qm);
	
	// store results to compute statistics in finalize step
	pccResults.push_back(std::make_pair(_context->getFrame(), qm));

	// 
	return 0;
}

// collect multi-frame statistics
void Compare::pccFinalize(void) {

	if (pccResults.size() > 1) {

		Statistics::Results stats;

		Statistics::compute(pccResults.size(),
			[&](size_t i) -> double { return pccResults[i].second.c2c_mse; },
			stats);
		
		Statistics::printToLog(stats, "mseF, PSNR(p2point) ", std::cout);

		Statistics::compute(pccResults.size(),
			[&](size_t i) -> double { return pccResults[i].second.c2p_mse; },
			stats);

		Statistics::printToLog(stats, "mseF, PSNR(p2plane) ", std::cout);

		Statistics::compute(pccResults.size(),
			[&](size_t i) -> double { return pccResults[i].second.color_psnr[0]; },
			stats);

		Statistics::printToLog(stats, "c[0],PSNRF          ", std::cout);

		Statistics::compute(pccResults.size(),
			[&](size_t i) -> double { return pccResults[i].second.color_psnr[1]; },
			stats);

		Statistics::printToLog(stats, "c[1],PSNRF          ", std::cout);

		Statistics::compute(pccResults.size(),
			[&](size_t i) -> double { return pccResults[i].second.color_psnr[2]; },
			stats);

		Statistics::printToLog(stats, "c[2],PSNRF          ", std::cout);

	}

}

// utility func used by compare::pcqm
// no sanity check, we assume the model is clean 
// and generated by sampling that allways generate content with color and normals
void convertModel(const Model& inputModel, PointSet& outputModel) {
	// init bbox boundaries
	outputModel.xmin = outputModel.ymin = outputModel.zmin = std::numeric_limits<double>::max();
	outputModel.xmax = outputModel.ymax = outputModel.zmax = std::numeric_limits<double>::min();
	// note that RGBA is not supported - no error checking
	const bool haveColors = inputModel.colors.size() == inputModel.vertices.size();
	// copy data
	for (size_t i = 0; i < inputModel.vertices.size() / 3; ++i) {
		Point point;
		// push the positions
		point.x = inputModel.vertices[i * 3];
		point.y = inputModel.vertices[i * 3 + 1];
		point.z = inputModel.vertices[i * 3 + 2];
		// push the colors if any
		// PointSet ingests RGB8 stored on double.
		if (haveColors) {
			point.r = (double)inputModel.colors[i * 3];
			point.g = (double)inputModel.colors[i * 3 + 1];
			point.b = (double)inputModel.colors[i * 3 + 2];
		}
		// PCQM needs valid color attributes we generate a pure white
		else {
			point.r = point.g = point.b = (double)255;
		}
		// will add the point and update bbox
		outputModel.pts.push_back( point );
		outputModel.xmax = outputModel.xmax > point.x ? outputModel.xmax : point.x;
		outputModel.ymax = outputModel.ymax > point.y ? outputModel.ymax : point.y;
		outputModel.zmax = outputModel.zmax > point.z ? outputModel.zmax : point.z;
		outputModel.xmin = outputModel.xmin < point.x ? outputModel.xmin : point.x;
		outputModel.ymin = outputModel.ymin < point.y ? outputModel.ymin : point.y;
		outputModel.zmin = outputModel.zmin < point.z ? outputModel.zmin : point.z;
	}
}

int Compare::pcqm(
	const Model& modelA, const Model& modelB,
	const Image& mapA, const Image& mapB,
	const double radiusCurvature,
	const int thresholdKnnSearch,
	const double radiusFactor,
	Model& outputA, Model& outputB,
	const std::string& topoMapFilenane
) {

	// 0 - check topology if needed
	if (topoMapFilenane.length() != 0) {
		if(checkTopology(topoMapFilenane, modelA, modelB))
		{
			std::cout<<"Given the mapping in "<<topoMapFilenane<<" the topologies correspond to each other"<<std::endl;
		} else {
			std::cout<<"Given the mapping in "<<topoMapFilenane<<" the topologies DO NOT correspond to each other"<<std::endl;
		}
	}

	// 1 - sample the models if needed
	sampleIfNeeded(modelA, mapA, outputA);
	sampleIfNeeded(modelB, mapB, outputB);

	// 2 - transcode to PCQM internal format
	PointSet inCloud1;
	PointSet inCloud2;

	convertModel(outputA, inCloud1);
	convertModel(outputB, inCloud2);

	// 3 - compute the metric
	// ModelA is Reference model 
	// switch ref anf deg as in original PCQM (order matters)
	double pcqm = compute_pcqm(inCloud2, inCloud1, "reffile", "regfile", radiusCurvature, thresholdKnnSearch, radiusFactor);

	// compute PSNR
	// we use outputA as reference for PSNR signal dynamic
	glm::vec3 minBox, maxBox;
	computeBBox(outputA.vertices, minBox, maxBox);
	const double maxEnergy = glm::length(maxBox - minBox);
	double maxPcqm = 1.0;
	double pcqmScaled = (pcqm / maxPcqm) * maxEnergy;
	double pcqmMse = pcqmScaled * pcqmScaled;
	double pcqmPsnr = 10.0 * log10(maxEnergy * maxEnergy / pcqmMse);

	std::cout << "PCQM-PSNR=" << pcqmPsnr << std::endl;

	// store results to compute statistics
	pcqmResults.push_back(std::make_tuple(_context->getFrame(), pcqm, pcqmPsnr));

	// 
	return 0;
}

// 
void Compare::pcqmFinalize(void) {

	if (pcqmResults.size() > 1) {

		Statistics::Results stats;

		Statistics::compute(pcqmResults.size(), 
			[&](size_t i) -> double { return std::get<1>(pcqmResults[i]); }, 
			stats);

		Statistics::printToLog(stats, "PCQM ", std::cout);

		Statistics::compute(pcqmResults.size(),
			[&](size_t i) -> double { return std::get<2>(pcqmResults[i]); },
			stats);

		Statistics::printToLog(stats, "PCQM-PSNR ", std::cout);
		
	}

}

bool Compare::checkTopology(const std::string& topoMapFilenane, const Model& modelA, const Model& modelB) {
	
	// 1 - Test if number of triangles of output matches input number of triangles
	if (modelA.getTriangleCount() != modelB.getTriangleCount()) {
		std::cout << "Topologies are different: number of triangles differs (A=" << 
			modelA.getTriangleCount() << ",B=" << modelB.getTriangleCount() << ").";
		return false;
	}

	// 2 - parse the topomap and test bijection

	// where to store the topo map,
	// topoMap[dest face index] = source face index
	std::vector< size_t > topoMap(modelA.getTriangleCount(), 0);
	// did we already set one association for teh given index
	std::vector< bool > visited(modelA.getTriangleCount(), false);

	//
	std::ifstream topoFile;
	topoFile.open(topoMapFilenane.c_str(), std::ios::in);
	if (!topoFile)
	{
		std::cerr << "Error: can't open topology mapping file " << topoMapFilenane << std::endl;
		return false;
	}
	// file parsing
	size_t lineNo=0;
	std::string line;
	std::getline(topoFile, line);
	while (topoFile) {
		// parse the line
		std::istringstream in(line);
		size_t faces[2]; 
		for (size_t index = 0; index < 2; ++index) {
			if (! (in >> faces[index]) ) {
				std::cerr << "Error: " << topoMapFilenane << ":" << lineNo 
					<< " missing face number " << index << std::endl;
				return false;
			}
			if (faces[index] >= modelA.getTriangleCount()) {
				std::cerr << "Error: " << topoMapFilenane << ":" << lineNo 
					<< " face index out of range (faces[" << index << "]=" << faces[index]
					<<") >= (modelA.getTriangleCount()=" << modelA.getTriangleCount() << ")" << std::endl;
				return false;
			}
		}
		if (visited[faces[0]]) {
			std::cerr << "Error: " << topoMapFilenane << ":" << lineNo 
				<< " modelB face " << faces[0] << " already associated with modelA face " << topoMap[faces[0]] << std::endl;
			return false;
		}
		visited[faces[0]] = true;
		topoMap[faces[0]] = faces[1];
		// read next line
		std::getline(topoFile, line);
		lineNo++;
	}
	topoFile.close();

	// we already know that modelA and modelA tri count are equal
	// and that association map did not contain out of range indices
	// and that each association was unique
	// so just need to check that every entry has an association
	bool bijective = true;
	for (size_t index = 0; index < visited.size(); ++index) {
		bijective = bijective && visited[index] == 1;
	}
	if (!bijective) {
		std::cout << "Topologies are different: topology map is not bijective."<< std::endl;
		return false;
	}

	// 3 - Test if each output triangle respects the orientation of its associated input triangle
	// recall that modelA and modelB trianglecount are equals
	// we want cosine of angle made by both normals to be positive
	// Note:
	// - our orientation check does not only rely on the topology indexes but also on the positions (together with their distortion)
	// - are there cases where quantization might flip some faces ? Don't know.
	// - this approach migh t not be fully robust, to be cross checked and discussed.
	for (size_t triIdx = 0; triIdx < modelB.getTriangleCount(); ++triIdx) {
		glm::vec3 av1, av2, av3, bv1, bv2, bv3;
		modelA.fetchTriangleVertices(topoMap[triIdx], av1, av2, av3);
		modelB.fetchTriangleVertices(triIdx, bv1, bv2, bv3);
		glm::vec3 an, bn;
		triangleNormal(av1, av2, av3, an);
		triangleNormal(bv1, bv2, bv3, bn);
		float dot = glm::dot(an, bn);
		if ( dot < 0.0f ){
			std::cout << "Topologies are different: orientations are not preserved ("<< dot <<" < 0.0)."<< std::endl;
			std::cout << "Triangle["<<topoMap[triIdx]<<"] = [("<< av1[0] <<","<< av1[1] <<","<< av1[2] <<"),("<< av2[0] <<","<< av2[1] <<","<< av2[2] <<"),("<< av3[0] <<","<< av3[1] <<","<< av3[2] <<")] and normal ("<< an[0] <<","<< an[1] <<","<< an[2] <<")."<< std::endl;
			std::cout << "Triangle["<<triIdx<<"] = [("<< bv1[0] <<","<< bv1[1] <<","<< bv1[2] <<"),("<< bv2[0] <<","<< bv2[1] <<","<< bv2[2] <<"),("<< bv3[0] <<","<< bv3[1] <<","<< bv3[2] <<")] and normal ("<< bn[0] <<","<< bn[1] <<","<< bn[2] <<")."<< std::endl;
			return false;
		};
	}
	//
	return true;
}