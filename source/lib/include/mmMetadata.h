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

#ifndef _MM_METADATA_H_
#define _MM_METADATA_H_

#include <algorithm>  // for std::min and std::max
#include <cmath>      // for pow and sqrt,
#include <limits>     // for nan
#include <fstream>
#include <vector>
// mathematics
#include <glm/vec3.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

namespace mm {

class Metadata {
 public:
  Metadata() { _gridRes, _dilateSize, _paddingSize = 0; }

  Metadata( size_t gridRes, size_t dilateSize, size_t paddingSize ) {
    _gridRes     = gridRes;
    _dilateSize  = dilateSize;
    _paddingSize = paddingSize;
  }

  void write( std::string name ) {
    std::string metdataFileName = std::filesystem::path( name ).parent_path().string() + "/" +
                                  std::filesystem::path( name ).stem().string() + ".meta";

    std::ofstream file( metdataFileName, std::ios::out );
    if ( !file.is_open() ) { return; }
    file << _gridRes << std::endl;
    file << _dilateSize << std::endl;
    file << _paddingSize << std::endl;
    file << _patchCoordinates.size() << std::endl;
    for ( auto& element : _patchCoordinates ) { file << element[0] << " " << element[1] << std::endl; }
  };
  void read( std::string name ) {
    std::string metdataFileName = std::filesystem::path( name ).parent_path().string() + "/" +
                                  std::filesystem::path( name ).stem().string() + ".meta";

    int           patchCoordinatesSize = 0;
    std::ifstream file( metdataFileName, std::ios::in );
    if ( !file.is_open() ) { return; }
    file >> _gridRes;
    file >> _dilateSize;
    file >> _paddingSize;
    file >> patchCoordinatesSize;
    _patchCoordinates.resize( patchCoordinatesSize );
    for ( auto& element : _patchCoordinates ) { file >> element[0] >> element[1]; }
  };

  void print() {
    std::cout << "Metadata are:" << std::endl;
    std::cout << "  _gridRes:     " << getGridRes() << std::endl;
    std::cout << "  _dilateSize:  " << getDilateSize() << std::endl;
    std::cout << "  _paddingSize: " << getPaddingSize() << std::endl;
    std::cout << "  _patchCoordinates: ";
    for ( int i = 0; i < getPatchCoordinates().size(); ++i ) {
      std::cout << " [" << i << "]:" << getPatchCoordinates()[i].x << "/" << getPatchCoordinates()[i].y;
    }
    std::cout << " " << std::endl;
  }

  std::vector<glm::ivec2>& getPatchCoordinates() { return _patchCoordinates; };
  size_t&                  getGridRes() { return _gridRes; };
  size_t&                  getDilateSize() { return _dilateSize; };
  size_t&                  getPaddingSize() { return _paddingSize; };

 private:
  size_t                  _gridRes;
  size_t                  _dilateSize;   // dilate is used to prevent texture seams
  size_t                  _paddingSize;  // padding is used to get better compression
  std::vector<glm::ivec2> _patchCoordinates;
};

}  // namespace mm

#endif
