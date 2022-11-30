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
#include "mmGeometry.h"

#include "mmCmdNormals.h"

const char* CmdNormals::name  = "normals";
const char* CmdNormals::brief = "Computes the mesh normals.";

//
Command* CmdNormals::create() { return new CmdNormals(); }

//
bool CmdNormals::initialize( Context* ctx, std::string app, int argc, char* argv[] ) {
  // command line parameters
  try {
    cxxopts::Options options( app + " " + name, brief );
    // clang-format off
		options.add_options()
			("i,inputModel", "path to input model (obj or ply file)",
				cxxopts::value<std::string>())
			("o,outputModel", "path to output model (obj or ply file)",
				cxxopts::value<std::string>())
			("h,help", "Print usage")
			("normalized", "generated normals are normalized",
				cxxopts::value<bool>()->default_value("true"))
			("noSeams", "if enabled generation is slower but vertex located on UV seams are properly used as one same vertex, hence removing lighting seams.",
				cxxopts::value<bool>()->default_value("true"))
				;
    // clang-format on

    auto result = options.parse( argc, argv );

    // Analyse the options
    if ( result.count( "help" ) || result.arguments().size() == 0 ) {
      std::cout << options.help() << std::endl;
      return false;
    }
    //
    if ( result.count( "inputModel" ) )
      _inputModelFilename = result["inputModel"].as<std::string>();
    else {
      std::cerr << "Error: missing inputModel parameter" << std::endl;
      std::cout << options.help() << std::endl;
      return false;
    }
    //
    if ( result.count( "outputModel" ) )
      _outputModelFilename = result["outputModel"].as<std::string>();
    else {
      std::cerr << "Error: missing outputModel parameter" << std::endl;
      std::cout << options.help() << std::endl;
      return false;
    }
    //
    if ( result.count( "normalized" ) ) _normalized = result["normalized"].as<bool>();
    if ( result.count( "noSeams" ) ) _noSeams = result["noSeams"].as<bool>();
  } catch ( const cxxopts::OptionException& e ) {
    std::cout << "error parsing options: " << e.what() << std::endl;
    return false;
  }

  return true;
}

bool CmdNormals::process( uint32_t frame ) {
  // the input
  mm::Model* inputModel;
  if ( ( inputModel = mm::IO::loadModel( _inputModelFilename ) ) == NULL ) { return false; }
  if ( inputModel->vertices.size() == 0 ) {
    std::cout << "Error: invalid input model (missing vertices) from " << _inputModelFilename << std::endl;
    return false;
  }
  if ( inputModel->triangles.size() == 0 ) {
    std::cout << "Error: invalid input model (not a mesh) from " << _inputModelFilename << std::endl;
    return false;
  }

  // the output
  mm::Model* outputModel = new mm::Model();
  *outputModel           = *inputModel;

  // Perform the processings
  clock_t t1 = clock();

  std::cout << "Normals generation" << std::endl;
  std::cout << "  NoSeams = " << _noSeams << std::endl;
  std::cout << "  Normalized = " << _normalized << std::endl;

  outputModel->computeVertexNormals( _normalized, _noSeams );

  clock_t t2 = clock();
  std::cout << "Time on processing: " << ( (float)( t2 - t1 ) ) / CLOCKS_PER_SEC << " sec." << std::endl;

  // save the result
  if ( mm::IO::saveModel( _outputModelFilename, outputModel ) )
    return true;
  else {
    delete outputModel;
    return false;
  }

  return true;
}
