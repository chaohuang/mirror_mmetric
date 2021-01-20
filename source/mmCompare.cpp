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

const char* Compare::brief = "Compare model A vs model B";

//
int Compare::main(std::string app, std::string cmd, int argc, char* argv[])
{
	// the command options
	std::string inputModelAFilename, inputModelBFilename;
	std::string inputTextureAFilename, inputTextureBFilename;
	std::string outputModelAFilename, outputModelBFilename;
	// the type of processing
	std::string mode = "equ";
	// Equ options
	float epsilon = 0;
	// Pcc options
	pcc_quality::commandPar params;
	params.singlePass = false;
	params.hausdorff = false;
	params.bColor = true;
	params.bLidar = false; // allways false, no option
	params.resolution = 0.0; // auto
	params.neighborsProc = 1;
	params.dropDuplicates = 2;
	params.bAverageNormals = false;
	// PCQM options
	double radiusCurvature = 0.001;
	int thresholdKnnSearch = 20;
	double radiusFactor = 2.0;

	// command line parameters
	try
	{
		cxxopts::Options options(app + " compare", Compare::brief);
		options.add_options()
			("inputModelA", "path to input model A (obj or ply file)",
				cxxopts::value<std::string>())
			("inputModelB", "path to input model B (obj or ply file)",
				cxxopts::value<std::string>())
			("inputMapA", "path to input texture map A (png, jpeg)",
				cxxopts::value<std::string>())
			("inputMapB", "path to input texture map B (png, jpeg)",
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
			("epsilon", "floating point value, error threshold in equality comparison. if not set use memcmp.",
				cxxopts::value<float>()->default_value("0.0"))
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
				cxxopts::value<bool>()->default_value("false"))
			;
		options.add_options("pcqm mode")
			("radiusCurvature", "Set a radius for the construction of the neighborhood. As the bounding box is already computed with this program, use proposed value.",
				cxxopts::value<double>()->default_value("0.001"))
			("thresholdKnnSearch", "Set the number of points used for the quadric surface construction",
				cxxopts::value<int>()->default_value("20"))
			("radiusFactor", "Set a radius factor for the statistic computation.",
				cxxopts::value<double>()->default_value("2.0"))
			;

		auto result = options.parse(argc, argv);

		// Analyse the options
		if (result.count("help") || result.arguments().size() == 0)
		{
			std::cout << options.help() << std::endl;
			return 0;
		}
		//	
		if (result.count("inputModelA"))
			inputModelAFilename = result["inputModelA"].as<std::string>();
		else {
			std::cerr << "Error: missing inputModelA parameter" << std::endl;
			std::cout << options.help() << std::endl;
			return 2;
		}
		//	
		if (result.count("inputModelB"))
			inputModelBFilename = result["inputModelB"].as<std::string>();
		else {
			std::cerr << "Error: missing inputModelB parameter" << std::endl;
			std::cout << options.help() << std::endl;
			return 2;
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
		//
		if (result.count("epsilon"))
			epsilon = result["epsilon"].as<float>();
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
		// PCSM
		if (result.count("radiusCurvature"))
			radiusCurvature = result["radiusCurvature"].as<double>();
		if (result.count("thresholdKnnSearch"))
			thresholdKnnSearch = result["thresholdKnnSearch"].as<int>();
		if (result.count("radiusFactor"))
			radiusFactor = result["radiusFactor"].as<double>();
	}
	catch (const cxxopts::OptionException& e)
	{
		std::cout << "error parsing options: " << e.what() << std::endl;
		return 2;
	}

	// Reading map if needed
	Image textureMapA, textureMapB;
	bool perVertexColor = false;
	if (inputTextureAFilename != "") {
		IO::loadImage(inputTextureAFilename, textureMapA);
	}
	else {
		std::cout << "Skipping map read, will parse use vertex color if any" << std::endl;
		perVertexColor = true;
	}
	if (inputTextureBFilename != "") {
		IO::loadImage(inputTextureBFilename, textureMapB);
		if (perVertexColor) {
			std::cout << "error: inputs model colors are not homogeneous " << std::endl;
			return 2;
		}
	}
	else {
		std::cout << "Skipping map read, will parse use vertex color if any" << std::endl;
		perVertexColor = true;
	}

	// the input
	Model inputModelA, inputModelB;
	if (!IO::loadModel(inputModelAFilename, inputModelA)) {
		return 2;
	}
	if (inputModelA.vertices.size() == 0) {
		std::cout << "Error: input model from " << inputModelAFilename << " has no vertices" << std::endl;
		return 2;
	}
	if (!IO::loadModel(inputModelBFilename, inputModelB) || inputModelB.vertices.size() == 0) {
		return 2;
	}
	if (inputModelB.vertices.size() == 0) {
		std::cout << "Error: input model from " << inputModelBFilename << " has no vertices" << std::endl;
		return 2;
	}

	// the output models if any
	Model outputModelA, outputModelB;
	int res = 2;

	// Perform the processings
	clock_t t1 = clock();
	if (mode == "equ") {
		std::cout << "Compare models for equality" << std::endl;
		std::cout << "  Epsilon = " << epsilon << std::endl;
		res = Compare::equ(
			inputModelA, inputModelB,
			textureMapA, textureMapB,
			epsilon,
			outputModelA, outputModelB);
	}
	else if (mode == "pcc") {
		std::cout << "Compare models using MPEG PCC distortion metric" << std::endl;
		std::cout << "  singlePass = " << params.singlePass << std::endl;
		std::cout << "  hausdorff = " << params.hausdorff << std::endl;
		std::cout << "  color = " << params.bColor << std::endl;
		std::cout << "  resolution = " << params.resolution << std::endl;
		std::cout << "  neighborsProc = " << params.neighborsProc << std::endl;
		std::cout << "  dropDuplicates = " << params.dropDuplicates << std::endl;
		std::cout << "  AverageNormals = " << params.bAverageNormals << std::endl;
		res = Compare::pcc(
			inputModelA, inputModelB,
			textureMapA, textureMapB, params,
			outputModelA, outputModelB);
	}
	else if (mode == "pcqm") {
		std::cout << "Compare models using PCQM distortion metric" << std::endl;
		std::cout << "  radiusCurvature = " << radiusCurvature << std::endl;
		std::cout << "  thresholdKnnSearch = " << thresholdKnnSearch << std::endl;
		std::cout << "  radiusFactor = " << radiusFactor << std::endl;
		res = Compare::pcqm(
			inputModelA, inputModelB,
			textureMapA, textureMapB,
			radiusCurvature,
			thresholdKnnSearch,
			radiusFactor,
			outputModelA, outputModelB);
	}
	else {
		std::cerr << "Error: invalid --mode " << mode << std::endl;
		return 2;
	}
	clock_t t2 = clock();
	std::cout << "Time on processing: " << ((float)(t2 - t1)) / CLOCKS_PER_SEC << " sec." << std::endl;

	// save the result
	if (outputModelAFilename != "") {
		if (!IO::saveModel(outputModelAFilename, outputModelA))
			return 2;
	}
	// save the result
	if (outputModelBFilename != "") {
		if (!IO::saveModel(outputModelBFilename, outputModelB))
			return 2;
	}
	// success
	std::cout << "return " << res << std::endl;
	return res;
}

int Compare::equ(
	const Model& inputA, const Model& inputB,
	const Image& mapA, const Image& mapB,
	float epsilon,
	Model& outputA, Model& outputB)
{
	//
	struct Point {
		float x;
		float y;
		float z;
		Point(float a, float b, float c) { x = a; y = b; z = c; };
		bool operator<(const Point& o) const {
			if (x != o.x) {
				return x < o.x;
			}
			if (y != o.y) {
				return y < o.y;
			}
			return z < o.z;
		}

	};

	// allocate room for the results
	outputA.vertices.resize(inputA.vertices.size());
	outputB.vertices.resize(inputB.vertices.size());

	// prepare outputA
	std::vector<Point> positions;

	for (int i = 0; i < inputA.vertices.size() / 3; i++) {
		Point point(inputA.vertices[i * 3 + 0], inputA.vertices[i * 3 + 1], inputA.vertices[i * 3 + 2]);
		positions.push_back(point);
	}
	std::sort(positions.begin(), positions.end());

	for (int i = 0; i < positions.size(); i++) {
		outputA.vertices[i * 3 + 0] = positions[i].x;
		outputA.vertices[i * 3 + 1] = positions[i].y;
		outputA.vertices[i * 3 + 2] = positions[i].z;
	}

	// prepare outputB
	positions.clear();
	for (int i = 0; i < inputB.vertices.size() / 3; i++) {
		Point point(inputB.vertices[i * 3 + 0], inputB.vertices[i * 3 + 1], inputB.vertices[i * 3 + 2]);
		positions.push_back(point);
	}
	std::sort(positions.begin(), positions.end());

	for (int i = 0; i < positions.size(); i++) {
		outputB.vertices[i * 3 + 0] = positions[i].x;
		outputB.vertices[i * 3 + 1] = positions[i].y;
		outputB.vertices[i * 3 + 2] = positions[i].z;
	}

	// now compare the results
	if (epsilon == 0) {
		if (outputB.vertices == outputA.vertices) {
			std::cout << "model vertices are equals" << std::endl;
			return 0;
		}
		else {
			std::cout << "model vertices are not equals" << std::endl;
			return 1;
		}
	}
	else {
		if (outputA.vertices.size() != outputB.vertices.size()) {
			std::cout << "model vertices are not equals" << std::endl;
			return 1;
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
		return (int)count;
	}
}

void sampleIfNeeded(
	const Model& input,
	const Image& map,
	Model& output)
{
	if (input.triangles.size() != 0) {
		// map method is better but require a minimum texture map size
		// otherwise we use sample face
		if (map.width >= 1024 && map.height >= 1024) {
			std::cout << "Sampling model with map method " << std::endl;
			Sample::meshToPcMap(input, output, map, false);
		}
		else {
			std::cout << "Sampling model with face method, resolution=" << 1024 << std::endl;
			Sample::meshToPcFace(input, output, map, 1024, 0.0, false, false);
		}
	}
	else {
		output = input; //  ugly copy, shall better use modelB directly
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
void convertModel(const Model& inputModel, pcc_quality::commandPar& params, PccPointCloud& outputModel ) {
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
	if ( inputModel.colors.size() == inputModel.vertices.size() )
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
	Model& outputA, Model& outputB
) {

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
	// compute color metric if valid color arrays
	params.bColor = params.bColor && (outputA.colors.size() == outputA.vertices.size() && outputB.colors.size() == outputB.vertices.size());

	// 3 - compute the metric
	pcc_quality::qMetric qm;
	computeQualityMetric(inCloud1, inCloud1, inCloud2, params, qm);

	// 
	return 0;
}

// utility func used by compare::pcqm
// no sanity check, we assume the model is clean 
// and generated by sampling that allways generate content with color and normals
void convertModel(const Model& inputModel, PointSet& outputModel) {
	// init bbox boundaries
	outputModel.xmin = outputModel.ymin = outputModel.zmin = std::numeric_limits<double>::max();
	outputModel.xmax = outputModel.ymax = outputModel.zmax = std::numeric_limits<double>::min();
	// copy data
	for (size_t i = 0; i < inputModel.vertices.size() / 3; ++i) {
		Point point;
		// push the positions
		point.x = inputModel.vertices[i * 3];
		point.y = inputModel.vertices[i * 3 + 1];
		point.z = inputModel.vertices[i * 3 + 2];
		// push the normals if any
		if (inputModel.normals.size() > i * 3 + 2) {
			point.nx = inputModel.normals[i * 3];
			point.ny = inputModel.normals[i * 3 + 1];
			point.nz = inputModel.normals[i * 3 + 2];
		}
		// push the colors if any
		if (inputModel.colors.size() > i * 3 + 2) {
			point.r = inputModel.colors[i * 3] / 256;
			point.g = inputModel.colors[i * 3 + 1] / 256;
			point.b = inputModel.colors[i * 3 + 2] / 256;
		}
		// will add the point and update bbox
		outputModel.addPoint(point);
	}
}

int Compare::pcqm(
	const Model& modelA, const Model& modelB,
	const Image& mapA, const Image& mapB,
	const double radiusCurvature,
	const int thresholdKnnSearch,
	const double radiusFactor,
	Model& outputA, Model& outputB
) {

	// 1 - sample the models if needed
	sampleIfNeeded(modelA, mapA, outputA);
	sampleIfNeeded(modelB, mapB, outputB);

	// 2 - transcode to PCQM internal format
	PointSet inCloud1;
	PointSet inCloud2;

	convertModel(outputA, inCloud1);
	convertModel(outputB, inCloud2);

	// 3 - compute the metric
	double pcqm = compute_pcqm(inCloud1, inCloud2, "reffile", "regfile", radiusCurvature, thresholdKnnSearch, radiusFactor);

	// compute PSNR
	double maxPcqm = 1.0;
	const double maxEnergy = std::numeric_limits<unsigned short>::max();
	double pcqmScaled = (pcqm / maxPcqm) * maxEnergy;
	double pcqmMse = pcqmScaled * pcqmScaled;
	double pcqmPsnr = 10.0 * log10(maxEnergy * maxEnergy / pcqmMse);

	std::cout << "PCQM-PSNR=" << pcqmPsnr << std::endl;

	// 
	return 0;
}
