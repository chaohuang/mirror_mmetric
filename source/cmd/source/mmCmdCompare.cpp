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
#include "dmetric/source/pcc_processing.hpp"
#include "pcqm/pcqm.h"
// internal headers
#include "mmIO.h"
#include "mmModel.h"
#include "mmImage.h"
#include "mmSample.h"
#include "mmGeometry.h"
#include "mmColor.h"
#include "mmRendererSw.h"
#include "mmRendererHw.h"
#include "mmStatistics.h"
#include "mmCompare.h"
#include "mmCmdCompare.h"

// "implementation" done in mmRendererHW
#include <stb_image_write.h>

const char* CmdCompare::name  = "compare";
const char* CmdCompare::brief = "Compare model A vs model B";

//
Command* CmdCompare::create() { return new CmdCompare(); }

//
bool CmdCompare::initialize( Context* context, std::string app, int argc, char* argv[] ) {
  _context = context;

  // command line parameters
  try {
    cxxopts::Options options( app + " " + name, brief );
    // clang-format off
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
    // clang-format on

    auto result = options.parse( argc, argv );

    // Analyse the options
    if ( result.count( "help" ) || result.arguments().size() == 0 ) {
      std::cout << options.help() << std::endl;
      return false;
    }
    //
    if ( result.count( "inputModelA" ) )
      _inputModelAFilename = result["inputModelA"].as<std::string>();
    else {
      std::cerr << "Error: missing inputModelA parameter" << std::endl;
      std::cout << options.help() << std::endl;
      return false;
    }
    //
    if ( result.count( "inputModelB" ) )
      _inputModelBFilename = result["inputModelB"].as<std::string>();
    else {
      std::cerr << "Error: missing inputModelB parameter" << std::endl;
      std::cout << options.help() << std::endl;
      return false;
    }
    //
    if ( result.count( "outputCsv" ) ) _outputCsvFilename = result["outputCsv"].as<std::string>();

    // mode in equ,pcc,pcqm,topo. defaults to equ
    if ( result.count( "mode" ) ) {
      _mode = result["mode"].as<std::string>();
      if ( _mode != "equ" && _mode != "pcc" && _mode != "pcqm" && _mode != "topo" && _mode != "ibsm" ) {
        std::cerr << "Error: invalid --mode \"" << _mode << "\"" << std::endl;
        return false;
      }
    }
    // Optional input texture maps
    if ( result.count( "inputMapA" ) ) _inputTextureAFilename = result["inputMapA"].as<std::string>();
    if ( result.count( "inputMapB" ) ) _inputTextureBFilename = result["inputMapB"].as<std::string>();

    // Optional
    if ( result.count( "outputModelA" ) ) _outputModelAFilename = result["outputModelA"].as<std::string>();
    if ( result.count( "outputModelB" ) ) _outputModelBFilename = result["outputModelB"].as<std::string>();
    // eq
    if ( result.count( "epsilon" ) ) _equEpsilon = result["epsilon"].as<float>();
    if ( result.count( "earlyReturn" ) ) _equEarlyReturn = result["earlyReturn"].as<bool>();
    if ( result.count( "unoriented" ) ) _equUnoriented = result["unoriented"].as<bool>();
    // PCC
    if ( result.count( "singlePass" ) ) _pccParams.singlePass = result["singlePass"].as<bool>();
    if ( result.count( "hausdorff" ) ) _pccParams.hausdorff = result["hausdorff"].as<bool>();
    if ( result.count( "color" ) ) _pccParams.bColor = result["color"].as<bool>();
    if ( result.count( "resolution" ) ) _pccParams.resolution = result["resolution"].as<float>();
    if ( result.count( "dropDuplicates" ) ) _pccParams.dropDuplicates = result["dropDuplicates"].as<int>();
    if ( result.count( "neighborsProc" ) ) _pccParams.neighborsProc = result["neighborsProc"].as<int>();
    if ( result.count( "averageNormals" ) ) _pccParams.bAverageNormals = result["averageNormals"].as<bool>();
    // PCQM
    if ( result.count( "radiusCurvature" ) ) _pcqmRadiusCurvature = result["radiusCurvature"].as<double>();
    if ( result.count( "thresholdKnnSearch" ) ) _pcqmThresholdKnnSearch = result["thresholdKnnSearch"].as<int>();
    if ( result.count( "radiusFactor" ) ) _pcqmRadiusFactor = result["radiusFactor"].as<double>();
    // topo
    if ( result.count( "faceMapFile" ) ) _topoFaceMapFilename = result["faceMapFile"].as<string>();
    if ( result.count( "vertexMapFile" ) ) _topoVertexMapFilename = result["vertexMapFile"].as<string>();
    // Raster
    if ( result.count( "ibsmResolution" ) ) _ibsmResolution = result["ibsmResolution"].as<unsigned int>();
    if ( result.count( "ibsmCameraCount" ) ) _ibsmCameraCount = result["ibsmCameraCount"].as<unsigned int>();
    if ( result.count( "ibsmCameraRotation" ) ) {
      _ibsmCameraRotation = result["ibsmCameraRotation"].as<std::string>();
      if ( !parseVec3( _ibsmCameraRotation, _ibsmCamRotParams ) ) {
        std::cout << "Error: parsing --ibsmCameraRotation=\"" << _ibsmCameraRotation
                  << "\" expected three floats with space separator" << std::endl;
        return false;
      }
    }
    if ( result.count( "ibsmRenderer" ) ) _ibsmRenderer = result["ibsmRenderer"].as<std::string>();
    if ( _mode != "ibsm" && _ibsmRenderer != "sw_raster" && _ibsmRenderer != "gl12_raster" ) {
      std::cout << "error invalid renderer choice: " << _ibsmRenderer << std::endl;
      return false;
    }
    if ( result.count( "ibsmDisableCulling" ) ) _ibsmDisableCulling = result["ibsmDisableCulling"].as<bool>();
    if ( result.count( "ibsmDisableReordering" ) ) _ibsmDisableReordering = result["ibsmDisableReordering"].as<bool>();

    if ( result.count( "ibsmOutputPrefix" ) ) _ibsmOutputPrefix = result["ibsmOutputPrefix"].as<std::string>();
  } catch ( const cxxopts::OptionException& e ) {
    std::cout << "error parsing options: " << e.what() << std::endl;
    return false;
  }

  return true;
}

// func to be moved into a mmFileUtility.cpp/h or in IO.h
// open output text file in append mode with proper float precision
std::ofstream& openOutputFile( const std::string& fileName, std::ofstream& fileOut ) {
  // let's open in append write mode
  fileOut.open( fileName.c_str(), std::ios::out | std::ofstream::app );
  // this is mandatory to print floats with full precision
  fileOut.precision( std::numeric_limits<float>::max_digits10 );
  //
  return fileOut;
}

// func to be moved into a mmFileUtility.cpp/h or in IO.h
// this version will check file size in addition to opening the text file for output
std::ofstream& openOutputFile( const std::string& fileName, std::ofstream& fileOut, std::streamoff& fileSize ) {
  // create or open in append mode output csv if needed
  fileSize = 0;
  if ( fileName != "" ) {
    // check if csv file is empty, need to open in read mode
    std::ifstream fileIn;
    fileIn.open( fileName, std::ios::binary );
    if ( fileIn ) {
      fileIn.seekg( 0, std::ios::end );
      fileSize = fileIn.tellg();
      fileIn.close();
    }
    openOutputFile( fileName.c_str(), fileOut );
  }
  //
  return fileOut;
};

bool CmdCompare::process( uint32_t frame ) {
  // Reading map if needed
  mm::Image *textureMapA, *textureMapB;
  bool       perVertexColor = false;
  if ( _inputTextureAFilename != "" ) {
    textureMapA = mm::IO::loadImage( _inputTextureAFilename );
  } else {
    std::cout << "Skipping map read, will parse use vertex color if any" << std::endl;
    textureMapA    = new mm::Image();
    perVertexColor = true;
  }
  if ( _inputTextureBFilename != "" ) {
    textureMapB = mm::IO::loadImage( _inputTextureBFilename );
    if ( perVertexColor ) {
      std::cout << "Error: inputs model colors are not homogeneous " << std::endl;
      return false;
    }
  } else {
    std::cout << "Skipping map read, will parse use vertex color if any" << std::endl;
    textureMapB    = new mm::Image();
    perVertexColor = true;
  }

  // the input
  mm::Model *inputModelA, *inputModelB;
  if ( ( inputModelA = mm::IO::loadModel( _inputModelAFilename ) ) == NULL ) { return false; }
  if ( inputModelA->vertices.size() == 0 ) {
    std::cout << "Error: input model from " << _inputModelAFilename << " has no vertices" << std::endl;
    return false;
  }
  if ( ( inputModelB = mm::IO::loadModel( _inputModelBFilename ) ) == NULL ) { return false; }
  if ( inputModelB->vertices.size() == 0 ) {
    std::cout << "Error: input model from " << _inputModelBFilename << " has no vertices" << std::endl;
    return false;
  }

  // the output models if any
  mm::Model* outputModelA = new mm::Model();
  mm::Model* outputModelB = new mm::Model();
  int        res          = 2;

  // create or open in append mode output csv if needed
  std::streamoff csvFileLength = 0;
  std::ofstream  csvFileOut;
  openOutputFile( _outputCsvFilename, csvFileOut, csvFileLength );

  // Perform the processings
  clock_t t1 = clock();
  if ( _mode == "equ" ) {
    std::cout << "Compare models for equality" << std::endl;
    std::cout << "  Epsilon = " << _equEpsilon << std::endl;
    res = _compare.equ( *inputModelA, *inputModelB, *textureMapA, *textureMapB, _equEpsilon, _equEarlyReturn,
                            _equUnoriented, *outputModelA, *outputModelB );

    // print the stats
    if ( csvFileOut ) {
      // print the header if file is empty
      if ( csvFileLength == 0 ) {
        csvFileOut << "modelA;textureA;modelB;textureB;frame;epsilon;earlyReturn;unoriented;meshEquality;textureDiffs"
                   << std::endl;
      }
      // print stats
      csvFileOut << _inputModelAFilename << ";" << _inputTextureAFilename << ";" << _inputModelBFilename << ";"
                 << _inputTextureBFilename << ";" << frame << ";" << _equEpsilon << ";" << _equEarlyReturn << ";"
                 << _equUnoriented << ";"
                 << "TODO"
                 << "TODO" << std::endl;
      // done
      csvFileOut.close();
    }
  } else if ( _mode == "topo" ) {
    std::cout << "Compare models topology for equivalence" << std::endl;
    std::cout << "  faceMapFile = " << _topoFaceMapFilename << std::endl;
    std::string faceMapFilenameResolved = mm::IO::resolveName( _context->getFrame(), _topoFaceMapFilename );
    std::cout << "  vertexMapFile = " << _topoVertexMapFilename << std::endl;
    std::string vertexMapFilenameResolved = mm::IO::resolveName( _context->getFrame(), _topoVertexMapFilename );
    res = _compare.topo( *inputModelA, *inputModelB, faceMapFilenameResolved, vertexMapFilenameResolved );

    // print the stats
    if ( csvFileOut ) {
      // print the header if file is empty
      if ( csvFileLength == 0 ) {
        csvFileOut << "modelA;textureA;modelB;textureB;faceMap;vertexMap;frame;equivalence" << std::endl;
      }
      // print stats
      csvFileOut << _inputModelAFilename << ";" << _inputTextureAFilename << ";" << _inputModelBFilename << ";"
                 << _inputTextureBFilename << ";" << faceMapFilenameResolved << ";" << vertexMapFilenameResolved << ";"
                 << frame << ";"
                 << "TODO" << std::endl;
      // done
      csvFileOut.close();
    }
  } else if ( _mode == "pcc" ) {
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
    res = _compare.pcc( *inputModelA, *inputModelB, *textureMapA, *textureMapB, _pccParams, *outputModelA, *outputModelB );

    // print the stats
    // TODO add all parameters in the output
    if ( csvFileOut ) {
      // retrieve  metric results
      auto& frameResults = _compare.getPccResults();

      // print the header if file is empty
      if (csvFileLength == 0)
      {
        csvFileOut << "p_inputModelA;"
                   << "p_inputModelB;"
                   << "p_inputMapA;"
                   << "p_inputMapB;"
                   << "p_singlePass;"
                   << "p_hausdorff;"
                   << "p_color;"
                   << "p_resolution;"
                   << "p_neighborsProc;"
                   << "p_dropDuplicates;"
                   << "p_averageNormals;"
                   << "frame;"
                   << "resolution;"
                   << "c2c_psnr;"
                   << "haus_c2c_psnr;"
                   << "c2p_psnr;"
                   << "hausc2p_psnr;"
                   << "color_psnr[0];"
                   << "color_psnr[1];"
                   << "color_psnr[2];"
                   << "haus_rgb_psnr[0];"
                   << "haus_rgb_psnr[1];"
                   << "haus_rgb_psnr[2]" << std::endl;
      }
      // print stats
      csvFileOut << _inputModelAFilename << ";"                            // inputModelA
                 << _inputModelBFilename << ";"                            // inputModelB
                 << _inputTextureAFilename << ";"                          // inputMapA
                 << _inputTextureBFilename << ";"                          // inputMapB
                 << _pccParams.singlePass << ";"                           // singlePass
                 << _pccParams.hausdorff << ";"                            // hausdorff
                 << _pccParams.bColor << ";"                               // color
                 << paramsResolution << ";"                                // resolution
                 << _pccParams.neighborsProc << ";"                        // neighborsProc
                 << _pccParams.dropDuplicates << ";"                       // dropDuplicates
                 << _pccParams.bAverageNormals << ";"                      // averageNormals
                 << frame << ";"                                           // frame
                 << _pccParams.resolution << ";"                           // resolution
                 << frameResults.second.c2c_psnr << ";"                    // c2c_psnr
                 << frameResults.second.c2c_hausdorff_psnr << ";"          // haus_c2c_psnr
                 << frameResults.second.c2p_psnr << ";"                    // c2p_psnr 
                 << frameResults.second.c2p_hausdorff_psnr << ";"          // hausc2p_psnr 
                 << frameResults.second.color_psnr[0] << ";"               // color_psnr[0]
                 << frameResults.second.color_psnr[1] << ";"               // color_psnr[1]
                 << frameResults.second.color_psnr[2] << ";"               // color_psnr[2]
                 << frameResults.second.color_rgb_hausdorff_psnr[0] << ";" // haus_rgb_psnr[0]
                 << frameResults.second.color_rgb_hausdorff_psnr[1] << ";" // haus_rgb_psnr[1]
                 << frameResults.second.color_rgb_hausdorff_psnr[2]        // haus_rgb_psnr[2]
                 << std::endl;
      // done
      csvFileOut.close();
    }
  } else if ( _mode == "pcqm" ) {
    std::cout << "Compare models using PCQM distortion metric" << std::endl;
    std::cout << "  radiusCurvature = " << _pcqmRadiusCurvature << std::endl;
    std::cout << "  thresholdKnnSearch = " << _pcqmThresholdKnnSearch << std::endl;
    std::cout << "  radiusFactor = " << _pcqmRadiusFactor << std::endl;
    res = _compare.pcqm( *inputModelA, *inputModelB, *textureMapA, *textureMapB, _pcqmRadiusCurvature, _pcqmThresholdKnnSearch,
                _pcqmRadiusFactor, *outputModelA, *outputModelB );
    // print the stats
    // TODO add all parameters in the output
    if ( csvFileOut ) {
      // retrieve  metric results
      auto& frameResults = _compare.getPcqmResults();

      // print the header if file is empty
      if ( csvFileLength == 0 ) {
        csvFileOut << "p_inputModelA;p_inputModelB;p_inputMapA;p_inputMapB;"
                   << "p_radiusCurvature;p_thresholdKnnSearch;p_radiusFactor;"
                   << "frame;pcqm;pcqm_psnr" << std::endl;
      }
      // print stats
      csvFileOut << _inputModelAFilename << ";" << _inputModelBFilename << ";" << _inputTextureAFilename << ";"
                 << _inputTextureBFilename << ";" << _pcqmRadiusCurvature << ";" << _pcqmThresholdKnnSearch << ";"
                 << _pcqmRadiusFactor << ";" << frame << ";" << (double)std::get<1>( frameResults ) << ";"
                 << (double)std::get<2>( frameResults ) << std::endl;
      // done
      csvFileOut.close();
    }
  } else if ( _mode == "ibsm" ) {
    std::cout << "Compare models using IBSM distortion metric" << std::endl;
    std::cout << "  ibsmRenderer = " << _ibsmRenderer << std::endl;
    std::cout << "  ibsmCameraCount = " << _ibsmCameraCount << std::endl;
    std::cout << "  ibsmCameraRotation = " << _ibsmCameraRotation << std::endl;
    std::cout << "  ibsmResolution = " << _ibsmResolution << std::endl;
    std::cout << "  ibsmDisableCulling = " << _ibsmDisableCulling << std::endl;
    std::cout << "  ibsmOutputPrefix = " << _ibsmOutputPrefix << std::endl;

    res = _compare.ibsm( *inputModelA, *inputModelB, *textureMapA, *textureMapB, _ibsmDisableReordering,
                         _ibsmResolution, _ibsmCameraCount, _ibsmCamRotParams, _ibsmRenderer, _ibsmOutputPrefix,
                         _ibsmDisableCulling, *outputModelA, *outputModelB );

    // print the stats
    if ( csvFileOut ) {
      // retrieve  metric results
      auto& frameResults = _compare.getIbsmResults();
      
      // print the header if file is empty
      if ( csvFileLength == 0 ) {
        csvFileOut << "p_inputModelA;p_inputModelB;p_inputMapA;p_inputMapB;"
                   << "p_ibsmRenderer;p_ibsmCameraCount;p_ibsmCameraRotation;p_ibsmResolution;"
                   << "p_ibsmDisableCulling;p_ibsmOutputPrefix;"
                   << "frame;geo_psnr;rgb_psnr;r_psnr;g_psnr;b_psnr;"
                   << "yuv_psnr;y_psnr;u_psnr;v_psnr;processingTime" << std::endl;
      }
      // print stats
      csvFileOut << _inputModelAFilename << ";" << _inputModelBFilename << ";" << _inputTextureAFilename << ";"
                 << _inputTextureBFilename << ";" << _ibsmRenderer << ";" << _ibsmCameraCount << ";"
                 << _ibsmCameraRotation << ";" << _ibsmResolution << ";" << _ibsmDisableCulling << ";"
                 << _ibsmOutputPrefix << ";" << frame << ";" << frameResults.second.depthPSNR << ";"
                 << frameResults.second.rgbPSNR[3] << ";" << frameResults.second.rgbPSNR[0] << ";"
                 << frameResults.second.rgbPSNR[1] << ";" << frameResults.second.rgbPSNR[2] << ";"
                 << frameResults.second.yuvPSNR[3] << ";" << frameResults.second.yuvPSNR[0] << ";"
                 << frameResults.second.yuvPSNR[1] << ";" << frameResults.second.yuvPSNR[2] << ";"
                 << ( (float)( clock() - t1 ) ) / CLOCKS_PER_SEC << std::endl;
      // done
      csvFileOut.close();
    }
  } else {
    std::cerr << "Error: invalid --mode " << _mode << std::endl;
    return false;
  }
  clock_t t2 = clock();
  std::cout << "Time on processing: " << ( (float)( t2 - t1 ) ) / CLOCKS_PER_SEC << " sec." << std::endl;

  // save the result
  if ( _outputModelAFilename != "" ) {
    if ( !mm::IO::saveModel( _outputModelAFilename, outputModelA ) ) return false;
  } else {
    delete outputModelA;
  }
  // save the result
  if ( _outputModelBFilename != "" ) {
    if ( !mm::IO::saveModel( _outputModelBFilename, outputModelB ) ) return false;
  } else {
    delete outputModelB;
  }
  // success
  std::cout << "return " << res << std::endl;
  return true;
}

bool CmdCompare::finalize() {
  // Collect the statistics
  if ( _mode == "pcc" ) { _compare.pccFinalize(); }
  if ( _mode == "pcqm" ) { _compare.pcqmFinalize(); }
  if ( _mode == "ibsm" ) { _compare.ibsmFinalize(); }
  return true;
}