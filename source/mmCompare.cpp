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
#include <string>
#include <vector>
// mathematics
#include <glm/vec3.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/transform.hpp>
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
#include "mmColor.h"
#include "mmStatistics.h"
#include "mmRendererSw.h"
#include "mmRendererHw.h"

// "implementation" done in mmRendererHW
#include <stb_image_write.h>

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
			("outputCsv", "filename of the file where per frame statistics will append.",
				cxxopts::value<std::string>()->default_value(""))
			("mode", "the comparison mode in [equ,pcc,pcqm,topo,ibsm]",
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
		options.add_options("topo mode")
			("faceMapFile", "path to the topology text file matching modelB topology (face) to modelA topology (face).",
				cxxopts::value<std::string>())
			("vertexMapFile", "path to the topology text file matching modelB topology (vertex) to modelA topology (vertex).",
				cxxopts::value<std::string>())
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
		options.add_options("ibsm mode")
			("ibsmResolution", "Resolution of the image buffer.",
				cxxopts::value<unsigned int>()->default_value("2048"))
			("ibsmCameraCount", "Number of virtual cameras to be used per frame.",
				cxxopts::value<unsigned int>()->default_value("16"))
			("ibsmCameraRotation", "Three parameters of rotating the virtual camera positions: the polar angle, the azimuthal angle and the rotation magnitude",
                cxxopts::value<std::string>()->default_value("0.0 0.0 0.0"))
			("ibsmRenderer", "Use software or openGL 1.2 renderer. Value in [sw_raster, gl12_raster].",
				cxxopts::value<std::string>()->default_value("sw_raster"))
			("ibsmDisableCulling", "Set option to disable the backface culling.",
				cxxopts::value<bool>()->default_value("false"))
			("ibsmDisableReordering", "Set option to disable automatic oriented reordering of input meshes, can be usefull if already done previously to save very small execution time.",
				cxxopts::value<bool>()->default_value("false"))
			("ibsmOutputPrefix", "Set option with a proper prefix/path system to dump the color shots as png images (Warning, it is extremly time consuming to write the buffers, use only for debug).",
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
			_inputModelAFilename = result["inputModelA"].as<std::string>();
		else {
			std::cerr << "Error: missing inputModelA parameter" << std::endl;
			std::cout << options.help() << std::endl;
			return false;
		}
		//	
		if (result.count("inputModelB"))
			_inputModelBFilename = result["inputModelB"].as<std::string>();
		else {
			std::cerr << "Error: missing inputModelB parameter" << std::endl;
			std::cout << options.help() << std::endl;
			return false;
		}
		//
		if (result.count("outputCsv"))
			_outputCsvFilename = result["outputCsv"].as<std::string>();

		// mode in equ,pcc,pcqm,topo. defaults to equ
		if (result.count("mode")){
			_mode = result["mode"].as<std::string>();
			if (_mode != "equ" && _mode != "pcc" && _mode != "pcqm" && _mode != "topo" && _mode != "ibsm") {
				std::cerr << "Error: invalid --mode \"" << _mode << "\"" << std::endl;
				return false;
			}
		}
		// Optional input texture maps
		if (result.count("inputMapA"))
			_inputTextureAFilename = result["inputMapA"].as<std::string>();
		if (result.count("inputMapB"))
			_inputTextureBFilename = result["inputMapB"].as<std::string>();

		// Optional
		if (result.count("outputModelA"))
			_outputModelAFilename = result["outputModelA"].as<std::string>();
		if (result.count("outputModelB"))
			_outputModelBFilename = result["outputModelB"].as<std::string>();
		// eq
		if (result.count("epsilon"))
			_equEpsilon = result["epsilon"].as<float>();
		if (result.count("earlyReturn"))
			_equEarlyReturn = result["earlyReturn"].as<bool>();
		if (result.count("unoriented"))
			_equUnoriented = result["unoriented"].as<bool>();
		// PCC
		if (result.count("singlePass"))
			_pccParams.singlePass = result["singlePass"].as<bool>();
		if (result.count("hausdorff"))
			_pccParams.hausdorff = result["hausdorff"].as<bool>();
		if (result.count("color"))
			_pccParams.bColor = result["color"].as<bool>();
		if (result.count("resolution"))
			_pccParams.resolution = result["resolution"].as<float>();
		if (result.count("dropDuplicates"))
			_pccParams.dropDuplicates = result["dropDuplicates"].as<int>();
		if (result.count("neighborsProc"))
			_pccParams.neighborsProc = result["neighborsProc"].as<int>();
		if (result.count("averageNormals"))
			_pccParams.bAverageNormals = result["averageNormals"].as<bool>();
		// PCQM
		if (result.count("radiusCurvature"))
			_pcqmRadiusCurvature = result["radiusCurvature"].as<double>();
		if (result.count("thresholdKnnSearch"))
			_pcqmThresholdKnnSearch = result["thresholdKnnSearch"].as<int>();
		if (result.count("radiusFactor"))
			_pcqmRadiusFactor = result["radiusFactor"].as<double>();
		// topo
		if (result.count("faceMapFile"))
			_topoFaceMapFilename = result["faceMapFile"].as<string>();
		if (result.count("vertexMapFile"))
			_topoVertexMapFilename = result["vertexMapFile"].as<string>();
		// Raster
		if (result.count("ibsmResolution"))
			_ibsmResolution = result["ibsmResolution"].as<unsigned int>();
		if (result.count("ibsmCameraCount"))
			_ibsmCameraCount = result["ibsmCameraCount"].as<unsigned int>();
		if (result.count("ibsmCameraRotation")) {
            _ibsmCameraRotation = result["ibsmCameraRotation"].as<std::string>();
            if (!parseVec3(_ibsmCameraRotation, CamRotParams)) {
                std::cout << "Error: parsing --ibsmCameraRotation=\"" << _ibsmCameraRotation << "\" expected three floats with space separator" << std::endl;
                return false;
            }
        }
		if (result.count("ibsmRenderer"))
			_ibsmRenderer = result["ibsmRenderer"].as<std::string>();
		if (_mode != "ibsm" && _ibsmRenderer != "sw_raster" && _ibsmRenderer != "gl12_raster") {
			std::cout << "error invalid renderer choice: " << _ibsmRenderer << std::endl;
			return false;
		}
		if (result.count("ibsmDisableCulling"))
			_ibsmDisableCulling = result["ibsmDisableCulling"].as<bool>();
		if (result.count("ibsmDisableReordering"))
			_ibsmDisableReordering = result["ibsmDisableReordering"].as<bool>();
		
		if (result.count("ibsmOutputPrefix"))
			_ibsmOutputPrefix = result["ibsmOutputPrefix"].as<std::string>();
	}
	catch (const cxxopts::OptionException& e)
	{
		std::cout << "error parsing options: " << e.what() << std::endl;
		return false;
	}

	// now initialize OpenGL contexts if needed
	// this part is valid for all the frames
	if (_mode == "ibsm" && _ibsmRenderer == "gl12_raster") {
		if (!_hwRenderer.initialize(_ibsmResolution, _ibsmResolution)) {
			return false;
		}
	}

	return true;
}

// func to be moved into a mmFileUtility.cpp/h or in IO.h
// open output text file in append mode with proper float precision
std::ofstream& openOutputFile(const std::string& fileName, std::ofstream& fileOut) {
	// let's open in append write mode
	fileOut.open(fileName.c_str(), std::ios::out | std::ofstream::app);
	// this is mandatory to print floats with full precision
	fileOut.precision(std::numeric_limits< float >::max_digits10);
	//
	return fileOut;
}

// func to be moved into a mmFileUtility.cpp/h or in IO.h
// this version will check file size in addition to opening the text file for output
std::ofstream& openOutputFile(const std::string& fileName, std::ofstream& fileOut, std::streamoff& fileSize) {

	// create or open in append mode output csv if needed
	fileSize = 0;
	if (fileName != "") {
		// check if csv file is empty, need to open in read mode
		std::ifstream fileIn;
		fileIn.open(fileName, std::ios::binary);
		if (fileIn) {
			fileIn.seekg(0, std::ios::end);
			fileSize = fileIn.tellg();
			fileIn.close();
		}
		openOutputFile(fileName.c_str(), fileOut);
	}
	//
	return fileOut;
};

bool Compare::process(uint32_t frame) {

	// Reading map if needed
	Image* textureMapA, * textureMapB;
	bool perVertexColor = false;
	if (_inputTextureAFilename != "") {
		textureMapA = IO::loadImage(_inputTextureAFilename);
	}
	else {
		std::cout << "Skipping map read, will parse use vertex color if any" << std::endl;
		textureMapA = new Image();
		perVertexColor = true;
	}
	if (_inputTextureBFilename != "") {
		textureMapB = IO::loadImage(_inputTextureBFilename);
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
	if ((inputModelA = IO::loadModel(_inputModelAFilename)) == NULL) {
		return false;
	}
	if (inputModelA->vertices.size() == 0) {
		std::cout << "Error: input model from " << _inputModelAFilename << " has no vertices" << std::endl;
		return false;
	}
	if ((inputModelB = IO::loadModel(_inputModelBFilename)) == NULL) {
		return false;
	}
	if (inputModelB->vertices.size() == 0) {
		std::cout << "Error: input model from " << _inputModelBFilename << " has no vertices" << std::endl;
		return false;
	}

	// the output models if any
	Model* outputModelA = new Model();
	Model* outputModelB = new Model();
	int res = 2;

	// create or open in append mode output csv if needed
	std::streamoff csvFileLength = 0;
	std::ofstream csvFileOut;
	openOutputFile(_outputCsvFilename, csvFileOut, csvFileLength);

	// Perform the processings
	clock_t t1 = clock();
	if (_mode == "equ") {
		std::cout << "Compare models for equality" << std::endl;
		std::cout << "  Epsilon = " << _equEpsilon << std::endl;
		res = Compare::equ(
			*inputModelA, *inputModelB,
			*textureMapA, *textureMapB,
			_equEpsilon, _equEarlyReturn, _equUnoriented,
			*outputModelA, *outputModelB);
		
		// print the stats
		if (csvFileOut) {
			// print the header if file is empty
			if (csvFileLength == 0) {
				csvFileOut << "modelA;textureA;modelB;textureB;frame;epsilon;earlyReturn;unoriented;meshEquality;textureDiffs" << std::endl;
			}
			// print stats
			csvFileOut 
				<< _inputModelAFilename << ";" << _inputTextureAFilename << ";" 
				<< _inputModelBFilename << ";" << _inputTextureBFilename << ";" 
				<< frame << ";" << _equEpsilon << ";" << _equEarlyReturn << ";" << _equUnoriented << ";"
				<< "TODO" << "TODO" << std::endl;
			// done
			csvFileOut.close();
		}
	}
	else if (_mode == "topo") {
		std::cout << "Compare models topology for equivalence" << std::endl;
		std::cout << "  faceMapFile = " << _topoFaceMapFilename << std::endl;
		std::string faceMapFilenameResolved = IO::resolveName(_context->getFrame(), _topoFaceMapFilename);
		std::cout << "  vertexMapFile = " << _topoVertexMapFilename << std::endl;
		std::string vertexMapFilenameResolved = IO::resolveName(_context->getFrame(), _topoVertexMapFilename);
		res = Compare::topo(
			*inputModelA, *inputModelB, 
			faceMapFilenameResolved, vertexMapFilenameResolved);
		
		// print the stats
		if (csvFileOut) {
			// print the header if file is empty
			if (csvFileLength == 0) {
				csvFileOut << "modelA;textureA;modelB;textureB;faceMap;vertexMap;frame;equivalence" << std::endl;
			}
			// print stats
			csvFileOut
				<< _inputModelAFilename << ";" << _inputTextureAFilename << ";"
				<< _inputModelBFilename << ";" << _inputTextureBFilename << ";"
				<< faceMapFilenameResolved << ";" << vertexMapFilenameResolved << ";"
				<< frame << ";" << "TODO" << std::endl;
			// done
			csvFileOut.close();
		}
	}
	else if (_mode == "pcc") {
		std::cout << "Compare models using MPEG PCC distortion metric" << std::endl;
		std::cout << "  singlePass = " << _pccParams.singlePass << std::endl;
		std::cout << "  hausdorff = " << _pccParams.hausdorff << std::endl;
		std::cout << "  color = " << _pccParams.bColor << std::endl;
		std::cout << "  resolution = " << _pccParams.resolution << std::endl;
		std::cout << "  neighborsProc = " << _pccParams.neighborsProc << std::endl;
		std::cout << "  dropDuplicates = " << _pccParams.dropDuplicates << std::endl;
		std::cout << "  averageNormals = " << _pccParams.bAverageNormals << std::endl;
		// just backup for logging because it might be modified by pcc function call if auto mode
		float paramsResolution = _pccParams.resolution;
		res = Compare::pcc(
			*inputModelA, *inputModelB,
			*textureMapA, *textureMapB, _pccParams,
			*outputModelA, *outputModelB);
		
		// print the stats
		// TODO add all parameters in the output
		if (csvFileOut) {
			// retrieve  metric results
			std::pair<uint32_t, pcc_quality::qMetric>& frameResults = _pccResults[_pccResults.size()-1];
			// print the header if file is empty
			if (csvFileLength == 0) {
				csvFileOut 
					<< "p_inputModelA;p_inputModelB;p_inputMapA;p_inputMapB;"
					<< "p_singlePass;p_hausdorff;p_color;p_resolution;p_neighborsProc;p_dropDuplicates;p_averageNormals;"
					<< "frame;resolution;"
					<< "c2c_psnr;c2p_psnr;haus_c2c_psnr;hausc2p_psnr;"
					<< "color_psnr[0];color_psnr[1];color_psnr[2];"
					<< "haus_rgb_psnr[0];haus_rgb_psnr[1];haus_rgb_psnr[2]"  << std::endl;
			}
			// print stats
			csvFileOut
				<< _inputModelAFilename << ";" << _inputModelBFilename  << ";"
				<< _inputTextureAFilename << ";" << _inputTextureBFilename << ";"
				<< _pccParams.singlePass << ";" << _pccParams.hausdorff << ";" << _pccParams.bColor << ";"
				<< paramsResolution << ";" << _pccParams.neighborsProc << ";" << _pccParams.dropDuplicates << ";" << _pccParams.bAverageNormals << ";"
				<< frame << ";" << _pccParams.resolution << ";"
				<< frameResults.second.c2c_psnr << ";" << frameResults.second.c2c_hausdorff_psnr << ";"
				<< frameResults.second.c2p_psnr << ";" << frameResults.second.c2p_hausdorff_psnr << ";"
				<< frameResults.second.color_psnr[0] << ";"
				<< frameResults.second.color_psnr[1] << ";"
				<< frameResults.second.color_psnr[2] << ";"
				<< frameResults.second.color_rgb_hausdorff_psnr[0] << ";"
				<< frameResults.second.color_rgb_hausdorff_psnr[1] << ";"
				<< frameResults.second.color_rgb_hausdorff_psnr[2]  << std::endl;
			// done
			csvFileOut.close();
		}
	}
	else if (_mode == "pcqm") {
		std::cout << "Compare models using PCQM distortion metric" << std::endl;
		std::cout << "  radiusCurvature = " << _pcqmRadiusCurvature << std::endl;
		std::cout << "  thresholdKnnSearch = " << _pcqmThresholdKnnSearch << std::endl;
		std::cout << "  radiusFactor = " << _pcqmRadiusFactor << std::endl;
		res = Compare::pcqm(
			*inputModelA, *inputModelB,
			*textureMapA, *textureMapB,
			_pcqmRadiusCurvature,
			_pcqmThresholdKnnSearch,
			_pcqmRadiusFactor,
			*outputModelA, *outputModelB);
		// print the stats
		// TODO add all parameters in the output
		if (csvFileOut) {
			// retrieve  metric results
			std::tuple<uint32_t, double, double >& frameResults = _pcqmResults[_pcqmResults.size() - 1];
			// print the header if file is empty
			if (csvFileLength == 0) {
				csvFileOut 
					<< "p_inputModelA;p_inputModelB;p_inputMapA;p_inputMapB;"
					<< "p_radiusCurvature;p_thresholdKnnSearch;p_radiusFactor;"
					<< "frame;pcqm;pcqm_psnr" << std::endl;
			}
			// print stats
			csvFileOut
				<< _inputModelAFilename << ";" << _inputModelBFilename << ";"
				<< _inputTextureAFilename << ";" << _inputTextureBFilename << ";"
				<< _pcqmRadiusCurvature << ";" << _pcqmThresholdKnnSearch << ";" << _pcqmRadiusFactor << ";"
				<< frame << ";" 
				<< (double)std::get<1>(frameResults) << ";"
				<< (double)std::get<2>(frameResults) << std::endl;
			// done
			csvFileOut.close();
		}
	}
	else if (_mode == "ibsm") {
		std::cout << "Compare models using IBSM distortion metric" << std::endl;
		std::cout << "  ibsmRenderer = " << _ibsmRenderer << std::endl;
		std::cout << "  ibsmCameraCount = " << _ibsmCameraCount << std::endl;
		std::cout << "  ibsmCameraRotation = " << _ibsmCameraRotation << std::endl;
		std::cout << "  ibsmResolution = " << _ibsmResolution << std::endl;
		std::cout << "  ibsmDisableCulling = " << _ibsmDisableCulling << std::endl;
		std::cout << "  ibsmOutputPrefix = " << _ibsmOutputPrefix << std::endl;

		res = Compare::ibsm(
			*inputModelA, *inputModelB,
			*textureMapA, *textureMapB,
			*outputModelA, *outputModelB);

		// print the stats
		if (csvFileOut) {
			// retrieve  metric results
			std::pair<uint32_t, IbsmResults>& frameResults = _ibsmResults[_ibsmResults.size() - 1];
			// print the header if file is empty
			if (csvFileLength == 0) {
				csvFileOut
					<< "p_inputModelA;p_inputModelB;p_inputMapA;p_inputMapB;"
					<< "p_ibsmRenderer;p_ibsmCameraCount;p_ibsmCameraRotation;p_ibsmResolution;"
					<< "p_ibsmDisableCulling;p_ibsmOutputPrefix;"
					<< "frame;geo_psnr;rgb_psnr;r_psnr;g_psnr;b_psnr;"
					<< "yuv_psnr;y_psnr;u_psnr;v_psnr;processingTime" << std::endl;
			}
			// print stats
			csvFileOut
				<< _inputModelAFilename << ";" << _inputModelBFilename << ";"
				<< _inputTextureAFilename << ";" << _inputTextureBFilename << ";"
				<< _ibsmRenderer << ";" << _ibsmCameraCount << ";" << _ibsmCameraRotation << ";" << _ibsmResolution << ";"
				<< _ibsmDisableCulling << ";" << _ibsmOutputPrefix << ";" 
				<< frame << ";"
				<< frameResults.second.depthPSNR << ";" 
				<< frameResults.second.rgbPSNR[3] << ";"
				<< frameResults.second.rgbPSNR[0] << ";"
				<< frameResults.second.rgbPSNR[1] << ";"
				<< frameResults.second.rgbPSNR[2] << ";"
				<< frameResults.second.yuvPSNR[3] << ";"
				<< frameResults.second.yuvPSNR[0] << ";"
				<< frameResults.second.yuvPSNR[1] << ";"
				<< frameResults.second.yuvPSNR[2] << ";"
				<< ((float)(clock() - t1)) / CLOCKS_PER_SEC << std::endl;
			// done
			csvFileOut.close();
		}
	}
	else {
		std::cerr << "Error: invalid --mode " << _mode << std::endl;
		return false;
	}
	clock_t t2 = clock();
	std::cout << "Time on processing: " << ((float)(t2 - t1)) / CLOCKS_PER_SEC << " sec." << std::endl;

	// save the result
	if (_outputModelAFilename != "") {
		if (!IO::saveModel(_outputModelAFilename, outputModelA))
			return false;
	}
	else {
		delete outputModelA;
	}
	// save the result
	if (_outputModelBFilename != "") {
		if (!IO::saveModel(_outputModelBFilename, outputModelB))
			return false;
	}
	else {
		delete outputModelB;
	}
	// success
	std::cout << "return " << res << std::endl;
	return true;
}

bool Compare::finalize() {

	// Collect the statistics
	if (_mode == "pcc") {
		pccFinalize();
	}
	
	if (_mode == "pcqm") {
		pcqmFinalize();
	}

	if (_mode == "ibsm") {
		
		ibsmFinalize();

		if (_ibsmRenderer == "gl12_raster") {
			return _hwRenderer.shutdown();
		}
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

inline bool areTrianglesEqual(bool unoriented,
	const Vertex& vA1, const Vertex& vA2, const Vertex& vA3,
	const Vertex& vB1, const Vertex& vB2, const Vertex& vB3)
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

int  Compare::topo(
	const Model& modelA, const Model& modelB,
	const std::string& faceMapFilename,
	const std::string& vertexMapFilename)
{
	// 1 - Test if number of triangles of output matches input number of triangles
	if (modelA.getTriangleCount() != modelB.getTriangleCount()) {
		std::cout << "Topologies are different: number of triangles differs (A=" <<
			modelA.getTriangleCount() << ",B=" << modelB.getTriangleCount() << ").";
		return false;
	}

	// 2 - parse the face map and test bijection

	// where to store the face map,
	// faceMap[dest face index] = source face index
	std::vector< size_t > faceMap(modelA.getTriangleCount(), 0);
	// did we already set one association for the given index
	std::vector< bool > visitedFace(modelA.getTriangleCount(), false);
	std::ifstream faceFile;
	faceFile.open(faceMapFilename.c_str(), std::ios::in);
	if (!faceFile)
	{
		std::cerr << "Error: can't open topology face mapping file " << faceMapFilename << std::endl;
		return false;
	}
	// file parsing
	size_t lineNo = 0;
	std::string line;
	std::getline(faceFile, line);
	while (faceFile) {
		// parse the line
		std::istringstream in(line);
		size_t faces[2];
		for (size_t index = 0; index < 2; ++index) {
			if (!(in >> faces[index])) {
				std::cerr << "Error: " << faceMapFilename << ":" << lineNo
					<< " missing face number " << index << std::endl;
				return false;
			}
			if (faces[index] >= modelA.getTriangleCount()) {
				std::cerr << "Error: " << faceMapFilename << ":" << lineNo
					<< " face index out of range (faces[" << index << "]=" << faces[index]
					<< ") >= (modelA.getTriangleCount()=" << modelA.getTriangleCount() << ")" << std::endl;
				return false;
			}
		}
		if (visitedFace[faces[0]]) {
			std::cerr << "Error: " << faceMapFilename << ":" << lineNo
				<< " modelB face " << faces[0] << " already associated with modelA face " << faceMap[faces[0]] << std::endl;
			return false;
		}
		visitedFace[faces[0]] = true;
		faceMap[faces[0]] = faces[1];
		// read next line
		std::getline(faceFile, line);
		lineNo++;
	}
	faceFile.close();

	// we already know that modelA and modelA tri count are equal
	// and that association map did not contain out of range indices
	// and that each association was unique
	// so just need to check that every entry has an association
	bool bijective = true;
	for (size_t index = 0; index < visitedFace.size(); ++index) {
		bijective = bijective && visitedFace[index] == 1;
	}
	if (!bijective) {
		std::cout << "Topologies are different: topology face map is not bijective." << std::endl;
		return false;
	}

	// 3 - parse the vertex map and test bijection

	// where to store the face map,
	// faceMap[dest face index] = source face index
	std::vector< size_t > vertexMap(modelA.getPositionCount(), 0);
	// did we already set one association for the given index
	std::vector< bool > visitedVertex(modelA.getPositionCount(), false);
	std::ifstream vertexFile;
	vertexFile.open(vertexMapFilename.c_str(), std::ios::in);
	if (!vertexFile)
	{
		std::cerr << "Error: can't open topology vertex mapping file " << vertexMapFilename << std::endl;
		return false;
	}
	// file parsing
	lineNo = 0;
	std::getline(vertexFile, line);
	while (vertexFile) {
		// parse the line
		std::istringstream in(line);
		size_t vertex[2];
		for (size_t index = 0; index < 2; ++index) {
			if (!(in >> vertex[index])) {
				std::cerr << "Error: " << vertexMapFilename << ":" << lineNo
					<< " missing vertex number " << index << std::endl;
				return false;
			}
			if (vertex[index] >= modelA.getPositionCount()) {
				std::cerr << "Error: " << vertexMapFilename << ":" << lineNo
					<< " vertex index out of range (vertex[" << index << "]=" << vertex[index]
					<< ") >= (modelA.getPositionCount()=" << modelA.getPositionCount() << ")" << std::endl;
				return false;
			}
		}
		if (visitedVertex[vertex[0]]) {
			std::cerr << "Error: " << vertexMapFilename << ":" << lineNo
				<< " modelB vertex " << vertex[0] << " already associated with modelA vertex " << vertexMap[vertex[0]] << std::endl;
			return false;
		}
		visitedVertex[vertex[0]] = true;
		vertexMap[vertex[0]] = vertex[1];
		// read next line
		std::getline(vertexFile, line);
		lineNo++;
	}
	vertexFile.close();

	// we already know that modelA and modelA tri count are equal
	// and that association map did not contain out of range indices
	// and that each association was unique
	// so just need to check that every entry has an association
	bijective = true;
	for (size_t index = 0; index < visitedVertex.size(); ++index) {
		bijective = bijective && visitedVertex[index] == 1;
	}
	if (!bijective) {
		std::cout << "Topologies are different: topology vertex map is not bijective." << std::endl;
		return false;
	}

	// 3 - Test if each output triangle respects the orientation of its associated input triangle
	// recall that modelA and modelB trianglecount are equals
	// we want the order of the vertices' indices to be the same or translated
	// that is ABC is equivalente to BCA and CAB, but not with ACB, BAC or CBA
	for (size_t triIdx = 0; triIdx < modelB.getTriangleCount(); ++triIdx) {
		int A1, A2, A3, B1, B2, B3;
		modelA.fetchTriangleIndices(faceMap[triIdx], A1, A2, A3);
		modelB.fetchTriangleIndices(triIdx, B1, B2, B3);
		size_t m1 = vertexMap[B1];
		size_t m2 = vertexMap[B2];
		size_t m3 = vertexMap[B3];
		if (!(((A1 == m1) && (A2 == m2) && (A3 == m3)) || ((A1 == m2) && (A2 == m3) && (A3 == m1)) || ((A1 == m3) && (A2 == m1) && (A3 == m2)))) {
			std::cout << "Topologies are different: orientations are not preserved " << std::endl;
			std::cout << "modelA->Triangle[" << faceMap[triIdx] << "] = [" << A1 << "," << A2 << "," << A3 << ")]." << std::endl;
			std::cout << "modelB(with mapped indices from modelA)->Triangle[" << triIdx << "] = [" << m1 << "," << m2 << "," << m3 << ")]." << std::endl;
			return false;
		};
	}

	std::cout << "Topologies are matching." << std::endl;

	return true;
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
	// compute color metric if valid color arrays (no support for RGBA colors)
	params.bColor = params.bColor && (outputA.colors.size() == outputA.vertices.size() && outputB.colors.size() == outputB.vertices.size());

	// 3 - compute the metric
	pcc_quality::qMetric qm;
	computeQualityMetric(inCloud1, inCloud1, inCloud2, params, qm);
	
	// store results to compute statistics in finalize step
	_pccResults.push_back(std::make_pair(_context->getFrame(), qm));

	// 
	return 0;
}

// collect multi-frame statistics
void Compare::pccFinalize(void) {

	if (_pccResults.size() > 1) {

		Statistics::Results stats;

		Statistics::compute(_pccResults.size(),
			[&](size_t i) -> double { return _pccResults[i].second.c2c_psnr; },
			stats);
		
		Statistics::printToLog(stats, "mseF, PSNR(p2point) ", std::cout);

		Statistics::compute(_pccResults.size(),
			[&](size_t i) -> double { return _pccResults[i].second.c2p_psnr; },
			stats);

		Statistics::printToLog(stats, "mseF, PSNR(p2plane) ", std::cout);

		Statistics::compute(_pccResults.size(),
			[&](size_t i) -> double { return _pccResults[i].second.color_psnr[0]; },
			stats);

		Statistics::printToLog(stats, "c[0],PSNRF          ", std::cout);

		Statistics::compute(_pccResults.size(),
			[&](size_t i) -> double { return _pccResults[i].second.color_psnr[1]; },
			stats);

		Statistics::printToLog(stats, "c[1],PSNRF          ", std::cout);

		Statistics::compute(_pccResults.size(),
			[&](size_t i) -> double { return _pccResults[i].second.color_psnr[2]; },
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
		outputModel.pts.push_back(point);
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
	_pcqmResults.push_back(std::make_tuple(_context->getFrame(), pcqm, pcqmPsnr));

	// 
	return 0;
}

// 
void Compare::pcqmFinalize(void) {

	if (_pcqmResults.size() > 1) {

		Statistics::Results stats;

		Statistics::compute(_pcqmResults.size(),
			[&](size_t i) -> double { return std::get<1>(_pcqmResults[i]); },
			stats);

		Statistics::printToLog(stats, "PCQM ", std::cout);

		Statistics::compute(_pcqmResults.size(),
			[&](size_t i) -> double { return std::get<2>(_pcqmResults[i]); },
			stats);

		Statistics::printToLog(stats, "PCQM-PSNR ", std::cout);

	}

}

// utility function to generate sample on a sphere
// used by the ibsm method
void fibonacciSphere(std::vector<glm::vec3>& points, int samples = 1, glm::vec3 rotParams = { 0.0F, 0.0F, 0.0F }) {

	bool rot = rotParams[2] ? true : false;
    glm::mat4 rotMatrix;

    if (rot) {
        float inclination = glm::radians(rotParams[0]);
        float azimuth = glm::radians(rotParams[1]);
        float intrinsic = glm::radians(rotParams[2]);
        float rotVec_r = std::abs(std::sin(inclination));
        float rotVec_y = std::cos(inclination);
        float rotVec_x = std::cos(azimuth) * rotVec_r;
        float rotVec_z = std::sin(azimuth) * rotVec_r;
        rotMatrix = glm::rotate(intrinsic, glm::vec3(rotVec_x, rotVec_y, rotVec_z));
    }
	
	const double pi = std::atan(1.0) * 4;

	// golden angle in radians
	float phi = (float)(pi * (3. - std::sqrt(5.)));

	for (size_t i = 0; i < samples; ++i) {

		float y = 1 - (i / float(samples - 1)) * 2;  // y goes from 1 to - 1
		float radius = std::sqrt(1 - y * y);  // radius at y
		float theta = phi * i; // golden angle increment

		float x = std::cos(theta) * radius;
		float z = std::sin(theta) * radius;

		glm::vec3 pos = {x, y, z};
		if (rot)
			pos = glm::vec3(rotMatrix * glm::vec4(pos, 1.0));

		points.push_back(pos);
	}
}

// compare two meshes using rasterization
int Compare::ibsm(
	const Model& modelA, const Model& modelB,
	const Image& mapA, const Image& mapB,
	Model& outputA, Model& outputB
) {
	
	// place for the results
	IbsmResults res;
	size_t maskSizeSum = 0; // store the sum for final computation of the mean

	clock_t t1 = clock();

	if (!_ibsmDisableReordering) {
		// reorder the faces if needed, reordering is important for metric stability and 
		// to get Infinite PSNR on equal meshes even with shuffled faces.
		reorder(modelA, "oriented", outputA);
		reorder(modelB, "oriented", outputB);
		std::cout << "Time on mesh reordering = " << ((float)(clock() - t1)) / CLOCKS_PER_SEC << " sec." << std::endl;
	}
	else {
		// just replicate the input
		// outputs may have updated normals hereafter
		outputA = modelA;
		outputB = modelB;
		std::cout << "Skipped reordering, time on mesh recopy = " << ((float)(clock() - t1)) / CLOCKS_PER_SEC << " sec." << std::endl;
	}

	const unsigned int width = _ibsmResolution;
	const unsigned int height = _ibsmResolution;
	glm::vec3 viewDir = { 0.0F, 0.0F, 1.0F };
	glm::vec3 viewUp = { 0.0F, 1.0F, 0.0F };
	glm::vec3 bboxMin;
	glm::vec3 bboxMax;

	// allocate frame buffer - will be cleared by renderer
	std::vector<uint8_t> fbufferRef(width * height * 4);
	std::vector<uint8_t> fbufferDis(width * height * 4);
	// allocate depth buffer - will be cleared by renderer
	std::vector<float> zbufferRef(width * height);
	std::vector<float> zbufferDis(width * height);
	// computes the overall bbox
	glm::vec3 refBboxMin, refBboxMax;
	glm::vec3 disBboxMin, disBboxMax;
	computeBBox(outputA.vertices, refBboxMin, refBboxMax, true);
	computeBBox(outputB.vertices, disBboxMin, disBboxMax, true);
	computeBBox(refBboxMin, refBboxMax, disBboxMin, disBboxMax, bboxMin, bboxMax);
	double refDiagLength = glm::length(refBboxMax - refBboxMin);
	double disDiagLength = glm::length(disBboxMax - disBboxMin);
	res.boxRatio = 100.0 * disDiagLength / refDiagLength;
	// default dynamic for Gl_raster, will be updated by sw_raster
	float sigDynamic = 1.0F;

	// prepare some camera directions
	std::vector<glm::vec3> camDir;
	fibonacciSphere(camDir, _ibsmCameraCount, CamRotParams);

	// for validation
	size_t depthNanCount = 0;
	size_t colorNanCount = 0;

	// now we render for each camera position
	for (size_t camIdx = 0; camIdx < camDir.size(); ++camIdx) {

		viewDir = camDir[camIdx];
		if (glm::distance(glm::abs(viewDir), glm::vec3(0,1,0)) < 1e-6)
			viewUp = glm::vec3(0, 0, 1);
		else
			viewUp = glm::vec3(0, 1, 0);

		std::cout << "render viewDir= " << viewDir[0] << " " << viewDir[1] << " " << viewDir[2] << std::endl;
		std::cout << "render viewUp= " << viewUp[0] << " " << viewUp[1] << " " << viewUp[2] << std::endl;

		clock_t t1 = clock();

		if (_ibsmRenderer == "gl12_raster") {

			if (_ibsmDisableCulling) _hwRenderer.disableCulling(); else _hwRenderer.enableCulling();

			_hwRenderer.render(&outputA, &mapA, fbufferRef, zbufferRef, width, height, viewDir, viewUp, bboxMin, bboxMax, true);
			_hwRenderer.render(&outputB, &mapB, fbufferDis, zbufferDis, width, height, viewDir, viewUp, bboxMin, bboxMax, true);
		}
		else {

			if (_ibsmDisableCulling) _swRenderer.disableCulling(); else	_swRenderer.enableCulling();

			_swRenderer.render(&outputA, &mapA, fbufferRef, zbufferRef, width, height, viewDir, viewUp, bboxMin, bboxMax, true);
			float depthRangeRef = _swRenderer.depthRange;

			_swRenderer.render(&outputB, &mapB, fbufferDis, zbufferDis, width, height, viewDir, viewUp, bboxMin, bboxMax, true);
			float depthRangeDis = _swRenderer.depthRange;

			if (depthRangeRef != depthRangeDis) { // should never occur
				std::cout << "Warning: reference and distorted signal dynamics are different, " 
					<< depthRangeRef << " vs " << depthRangeDis << std::endl;
			}
			sigDynamic = depthRangeDis;
			std::cout << "Signal Dynamic = " << depthRangeRef << std::endl;
		}

		clock_t t2 = clock();
		std::cout << "Time on buffers rendering: " << ((float)(t2 - t1)) / CLOCKS_PER_SEC << " sec." << std::endl;

		if (_ibsmOutputPrefix != "") {

			const std::string fullPrefix = _ibsmOutputPrefix + "_" +
				std::to_string(_context->getFrame()) + "_" + std::to_string(camIdx) + "_";

			// Write image Y-flipped because OpenGL
			stbi_write_png((fullPrefix + "ref.png").c_str(),
				width, height, 4,
				fbufferRef.data() + (width * 4 * (height - 1)),
				-(int)width * 4);

			// Write image Y-flipped because OpenGL
			stbi_write_png((fullPrefix + "dis.png").c_str(),
				width, height, 4,
				fbufferDis.data() + (width * 4 * (height - 1)),
				-(int)width * 4);
			
			// converts depth to positive 8 bit for visualization
			std::vector<uint8_t> zbufferRef_8bits(zbufferRef.size(), 255);
			std::vector<uint8_t> zbufferDis_8bits(zbufferRef.size(), 255);

			for (size_t i = 0; i < zbufferRef.size(); ++i) {
				if ( fbufferRef[i * 4 + 3] != 0 ){
					zbufferRef_8bits[i] = 255 - (uint8_t)(255 * (sigDynamic + zbufferRef[i]) / sigDynamic);
				}
				if (fbufferDis[i * 4 + 3] != 0) {
					zbufferDis_8bits[i] = 255 - (uint8_t)(255 * (sigDynamic + zbufferDis[i]) / sigDynamic);
				}
			}

			// Write image Y-flipped because OpenGL
			stbi_write_png((fullPrefix + "ref_depth.png").c_str(),
				width, height, 1,
				zbufferRef_8bits.data() + (width * 1 * (height - 1)),
				-(int)width * 1);

			// Write image Y-flipped because OpenGL
			stbi_write_png((fullPrefix + "dis_depth.png").c_str(),
				width, height, 1,
				zbufferDis_8bits.data() + (width * 1 * (height - 1)),
				-(int)width * 1);
		}

		// 0 - compute the amount of pixels where there is a projection of Ref or Dist
		for (size_t i = 0; i < fbufferRef.size() / 4; ++i) {
			const uint8_t maskRef = fbufferRef[i * 4 + 3];
			const uint8_t maskDis = fbufferDis[i * 4 + 3];
			if (maskRef != 0 || maskDis != 0) {
				maskSizeSum += 1;
			}
		}

		// A - now compute the Color Squared Error over the ref and dist images
		// store result in IbsmResults structures for convenience
		// but note that we store Squared Error into fields noted MSE
		for (size_t i = 0; i < fbufferRef.size() / 4; ++i) {
			const uint8_t maskRef = fbufferRef[i * 4 + 3];
			const uint8_t maskDis = fbufferDis[i * 4 + 3];
			// we are on non matching projection, we use full dynamic to emphasis the artefact
			if ((maskRef == 0 && maskDis != 0) || (maskRef != 0 && maskDis == 0)) {
				const double sse = (double)(255 * 255);
				for (size_t c = 0; c < 3; ++c) { // we skip the alpha channel
					res.rgbMSE[c] = res.rgbMSE[c] + sse;
					res.yuvMSE[c] = res.yuvMSE[c] + sse;
				}
			}
			// both object are projected on this pixel
			else if (maskRef != 0 && maskDis != 0) {
				// store YUV on vector of floats
				const glm::vec3 rgbRef(fbufferRef[i * 4 + 0], fbufferRef[i * 4 + 1], fbufferRef[i * 4 + 2]);
				const glm::vec3 rgbDis(fbufferDis[i * 4 + 0], fbufferDis[i * 4 + 1], fbufferDis[i * 4 + 2]);
				const glm::vec3 yuvRef = rgbToYuvBt709_256(rgbRef);
				const glm::vec3 yuvDis = rgbToYuvBt709_256(rgbDis);		
				// 
				for (glm::vec3::length_type c = 0; c < 3; ++c) { // we skip the alpha channel
					// |I1 - I2|
					double pixel_cmp_sse_rgb = (double)rgbRef[c] - (double)rgbDis[c];
					double pixel_cmp_sse_yuv = (double)yuvRef[c] - (double)yuvDis[c];
					// |I1 - I2|^2
					pixel_cmp_sse_rgb = pixel_cmp_sse_rgb * pixel_cmp_sse_rgb;
					pixel_cmp_sse_yuv = pixel_cmp_sse_yuv * pixel_cmp_sse_yuv;
					// ensures color values are valid, otherwise skip the sample
					if (std::isnan(pixel_cmp_sse_rgb) || std::isnan(pixel_cmp_sse_yuv)) {
						pixel_cmp_sse_rgb = 0.0;
						pixel_cmp_sse_yuv = 0.0;
						colorNanCount++;
					}
					// Sum mean
					res.rgbMSE[c] = res.rgbMSE[c] + pixel_cmp_sse_rgb;
					res.yuvMSE[c] = res.yuvMSE[c] + pixel_cmp_sse_yuv;
				}
			}
			// else we skip ~ add 0, because no pixel exist in both buffers (faster processing)
		}

		// B - now compute the Geometric MSE over the ref and dist depth buffers
		// allways renormalize on an energy range of 255x255 to be coherent with rgb PSNR
		// store result in IbsmResults structures for convenience
		// but note that we store Squared Error into fields noted MSE

		for (size_t i = 0; i < zbufferRef.size(); ++i) {
			const uint8_t maskRef = fbufferRef[i * 4 + 3];
			const uint8_t maskDis = fbufferDis[i * 4 + 3];
			// we are on non matching projection, we use rescaled full dynamic to emphasis the artefact
			if ((maskRef == 0 && maskDis != 0) || (maskRef != 0 && maskDis == 0)) {
				res.depthMSE = res.depthMSE + (double)(255 * 255);
			}
			// both object are projected on this pixel
			else if (maskRef != 0 && maskDis != 0) {
				// |I1 - I2|
				double pixel_depth_sse = ((double)zbufferRef[i] - (double)zbufferDis[i]) * 255.0 / sigDynamic;
				// |I1 - I2|^2
				pixel_depth_sse = pixel_depth_sse * pixel_depth_sse;
				// ensures depth values are valid, otherwise skip the sample
				if (std::isnan(pixel_depth_sse)) {
					pixel_depth_sse = 0.0;
					depthNanCount++;
				}
				// Sum mean
				res.depthMSE = res.depthMSE + pixel_depth_sse;
			}
			// else we skip ~ add 0, because no depth exist in both buffers (faster processing)
		}
		
		clock_t t3 = clock();
		std::cout << "Time on MSE computing: " << ((float)(t3 - t2)) / CLOCKS_PER_SEC << " sec." << std::endl;
	}

	// finally computes the MSE by dividing over total number of projected pixels
	for (size_t c = 0; c < 3; ++c) {
		res.rgbMSE[c] = res.rgbMSE[c] / (double)maskSizeSum;
		res.yuvMSE[c] = res.yuvMSE[c] / (double)maskSizeSum;
	}
	res.rgbMSE[3] = (res.rgbMSE[0] + res.rgbMSE[1] + res.rgbMSE[2]) / 3.0;
	res.yuvMSE[3] = (res.yuvMSE[0] * 6.0 + res.rgbMSE[1] + res.rgbMSE[2]) / 8.0; 
	res.depthMSE  = res.depthMSE / (double)maskSizeSum;

	// compute the PSNRs (can be infinite)
	for (size_t c = 0; c <= 3; ++c) {
		res.rgbPSNR[c] = std::min(999.99, 10.0 * log10((double)(255 * 255) / res.rgbMSE[c]));
		res.yuvPSNR[c] = std::min(999.99, 10.0 * log10((double)(255 * 255) / res.yuvMSE[c]));
	}
	res.depthPSNR = std::min(999.99, 10.0 * log10((double)(255 * 255) / res.depthMSE));

	// Debug
	if (depthNanCount != 0) {
		std::cout << "Warning: skipped " << depthNanCount << " NaN in depth buffer" << std::endl;
	}
	if (colorNanCount != 0) {
		std::cout << "Warning: skipped " << colorNanCount << " NaN in color buffer" << std::endl;
	}
	
	if (res.boxRatio < 99.5F || res.boxRatio > 100.5F ) {
		std::cout 
			<< "Warning: the size of the bounding box of reference and distorted models are quite different (see BoxRatio)."
			<< "  IBSM results might not be accurate. Please perform a visual check of your models." << std::endl;
	}

	// output the results
	std::cout << "BoxRatio = " << res.boxRatio << std::endl;
	std::cout << "R   MSE  = " << res.rgbMSE[0] << std::endl;
	std::cout << "G   MSE  = " << res.rgbMSE[1] << std::endl;
	std::cout << "B   MSE  = " << res.rgbMSE[2] << std::endl;
	std::cout << "RGB MSE  = " << res.rgbMSE[3] << std::endl;
	std::cout << "Y   MSE  = " << res.yuvMSE[0] << std::endl;
	std::cout << "U   MSE  = " << res.yuvMSE[1] << std::endl;
	std::cout << "V   MSE  = " << res.yuvMSE[2] << std::endl;
	std::cout << "YUV MSE  = " << res.yuvMSE[3] << std::endl;
	std::cout << "GEO MSE  = " << res.depthMSE << std::endl;
	std::cout << "R   PSNR = " << res.rgbPSNR[0] << std::endl;
	std::cout << "G   PSNR = " << res.rgbPSNR[1] << std::endl;
	std::cout << "B   PSNR = " << res.rgbPSNR[2] << std::endl;
	std::cout << "RGB PSNR = " << res.rgbPSNR[3] << std::endl;
	std::cout << "Y   PSNR = " << res.yuvPSNR[0] << std::endl;
	std::cout << "U   PSNR = " << res.yuvPSNR[1] << std::endl;
	std::cout << "V   PSNR = " << res.yuvPSNR[2] << std::endl;
	std::cout << "YUV PSNR = " << res.yuvPSNR[3] << std::endl;
	std::cout << "GEO PSNR = " << res.depthPSNR << std::endl;
	
	// store results to compute statistics
	_ibsmResults.push_back(std::make_pair(_context->getFrame(), res));

	return 0;
}

void Compare::ibsmFinalize(void) {

	if (_ibsmResults.size() > 1) {

		Statistics::Results stats;
		
		Statistics::compute(_ibsmResults.size(),
			[&](size_t i) -> double { return _ibsmResults[i].second.boxRatio; },
			stats);
		Statistics::printToLog(stats, "BoxRatio ", std::cout);

		Statistics::compute(_ibsmResults.size(),
			[&](size_t i) -> double { return _ibsmResults[i].second.rgbPSNR[3]; },
			stats);
		Statistics::printToLog(stats, "RGB PSNR ", std::cout);
		
		Statistics::compute(_ibsmResults.size(),
			[&](size_t i) -> double { return _ibsmResults[i].second.yuvPSNR[0]; },
			stats);
		Statistics::printToLog(stats, "Y   PSNR ", std::cout);

		Statistics::compute(_ibsmResults.size(),
			[&](size_t i) -> double { return _ibsmResults[i].second.yuvPSNR[1]; },
			stats);
		Statistics::printToLog(stats, "U   PSNR ", std::cout);

		Statistics::compute(_ibsmResults.size(),
			[&](size_t i) -> double { return _ibsmResults[i].second.yuvPSNR[2]; },
			stats);
		Statistics::printToLog(stats, "V   PSNR ", std::cout);

		Statistics::compute(_ibsmResults.size(),
			[&](size_t i) -> double { return _ibsmResults[i].second.yuvPSNR[3]; },
			stats);
		Statistics::printToLog(stats, "YUV PSNR ", std::cout);

		Statistics::compute(_ibsmResults.size(),
			[&](size_t i) -> double { return _ibsmResults[i].second.depthPSNR; },
			stats);
		Statistics::printToLog(stats, "GEO PSNR ", std::cout);
		
	}

}