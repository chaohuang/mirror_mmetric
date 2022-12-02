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
#include "mmColor.h"

using namespace mm;

//
void Dequantize::dequantize( const Model&     input,
                             Model&           output,
                             const uint32_t   qp,
                             const uint32_t   qt,
                             const uint32_t   qn,
                             const uint32_t   qc,
                             const glm::vec3& minPos,
                             const glm::vec3& maxPos,
                             const glm::vec2& minUv,
                             const glm::vec2& maxUv,
                             const glm::vec3& minNrm,
                             const glm::vec3& maxNrm,
                             const glm::vec3& minCol,
                             const glm::vec3& maxCol,
                             const bool       useFixedPoint,
                             const bool       colorSpaceConversion,
                             const bool       verbose ) {
  // copy input
  output = input;

  // dequantize position
  if ( !input.vertices.empty() && qp >= 7 ) {
    glm::vec3     minBox       = minPos;
    glm::vec3     maxBox       = maxPos;
    const int32_t fixedPoint16 = ( 1u << 16 );
    if ( useFixedPoint ) {
      // converting the values to a fixed point representation
      // minBox(FP16) will be used in AAPS -> shift
      for ( int i = 0; i < 3; i++ ) {
        if ( minBox[i] > 0 ) minBox[i] = ( std::floor( minBox[i] * fixedPoint16 ) ) / fixedPoint16;
        else minBox[i] = ( -1 ) * ( std::ceil( std::abs( minBox[i] ) * fixedPoint16 ) ) / fixedPoint16;
        if ( maxBox[i] > 0 ) maxBox[i] = ( std::ceil( maxBox[i] * fixedPoint16 ) ) / fixedPoint16;
        else maxBox[i] = ( -1 ) * ( std::floor( std::abs( maxBox[i] ) * fixedPoint16 ) ) / fixedPoint16;
      }
    }

    glm::vec3     diag              = maxBox - minBox;
    const float   range             = std::max( std::max( diag.x, diag.y ), diag.z );
    const int32_t maxQuantizedValue = ( 1u << static_cast<uint32_t>( qp ) ) - 1;
    float         scale             = range / maxQuantizedValue;
    if ( useFixedPoint ) scale = ( std::ceil( scale * fixedPoint16 ) ) / fixedPoint16;

    if ( verbose ) {
      std::cout << "  minPos=\"" << minBox.x << " " << minBox.y << " " << minBox.z << "\"" << std::endl;
      std::cout << "  maxPos=\"" << maxBox.x << " " << maxBox.y << " " << maxBox.z << "\"" << std::endl;
      std::cout << "  rangePos=" << range << std::endl;
      std::cout << "  maxQuantizedValue=" << maxQuantizedValue << std::endl;
      std::cout << "  scale=" << scale << std::endl;
    }

    for ( size_t i = 0; i < input.vertices.size() / 3; i++ ) {
      for ( glm::vec3::length_type c = 0; c < 3; ++c ) {
        // output.vertices[i * 3 + c] = (input.vertices[i * 3 + c] * range / maxQuantizedValue) + minBox[c];
        output.vertices[i * 3 + c] = ( input.vertices[i * 3 + c] * scale ) + minBox[c];
      }
    }
  }

  // dequantize UV coordinates
  if ( !input.uvcoords.empty() && qt >= 7 ) {
    const glm::vec2 minBox            = minUv;
    const glm::vec2 maxBox            = maxUv;
    const glm::vec2 diag              = maxBox - minBox;
    const float     range             = std::max( diag.x, diag.y );
    const int32_t   maxQuantizedValue = ( 1u << static_cast<uint32_t>( qt ) ) - 1;

    if ( verbose ) {
      std::cout << "  minUv=\"" << minBox.x << " " << minBox.y << "\"" << std::endl;
      std::cout << "  maxUv=\"" << maxBox.x << " " << maxBox.y << "\"" << std::endl;
      std::cout << "  rangeUv=" << range << std::endl;
    }

    for ( size_t i = 0; i < input.uvcoords.size() / 2; i++ ) {
      for ( glm::vec2::length_type c = 0; c < 2; ++c ) {
        output.uvcoords[i * 2 + c] = ( input.uvcoords[i * 2 + c] * range / maxQuantizedValue ) + minBox[c];
      }
    }
  }

  // dequantize normals
  if ( !input.normals.empty() && qn >= 7 ) {
    const glm::vec3 minBox            = minNrm;
    const glm::vec3 maxBox            = maxNrm;
    const glm::vec3 diag              = maxBox - minBox;
    const float     range             = std::max( std::max( diag.x, diag.y ), diag.z );
    const int32_t   maxQuantizedValue = ( 1u << static_cast<uint32_t>( qn ) ) - 1;

    if ( verbose ) {
      std::cout << "  minNrm=\"" << minBox.x << " " << minBox.y << " " << minBox.z << "\"" << std::endl;
      std::cout << "  maxNrm=\"" << maxBox.x << " " << maxBox.y << " " << maxBox.z << "\"" << std::endl;
      std::cout << "  rangeNrm=" << range << std::endl;
    }
    for ( size_t i = 0; i < input.normals.size() / 3; i++ ) {
      for ( glm::vec3::length_type c = 0; c < 3; ++c ) {
        output.normals[i * 3 + c] = ( input.normals[i * 3 + c] * range / maxQuantizedValue ) + minBox[c];
      }
    }
  }

  // dequantize colors
  if ( !input.colors.empty() && ( ( qc >= 7 ) || ( colorSpaceConversion ) ) ) {
    const glm::vec3 minBox            = minCol;
    const glm::vec3 maxBox            = maxCol;
    const glm::vec3 diag              = maxBox - minBox;
    const float     range             = std::max( std::max( diag.x, diag.y ), diag.z );
    const int32_t   maxQuantizedValue = ( 1u << static_cast<uint32_t>( qc ) ) - 1;

    if ( verbose ) {
      std::cout << "  minCol=\"" << minBox.x << " " << minBox.y << " " << minBox.z << "\"" << std::endl;
      std::cout << "  maxCol=\"" << maxBox.x << " " << maxBox.y << " " << maxBox.z << "\"" << std::endl;
      std::cout << "  rangeCol=" << range << std::endl;
    }
    for ( size_t i = 0; i < input.colors.size() / 3; i++ ) {
      if ( colorSpaceConversion ) {
        glm::vec3 inYUV, inYUV_256, inRGB_256;
        // vectorized by compiler
        for ( glm::vec3::length_type c = 0; c < 3; ++c ) { inYUV[c] = ( input.colors[i * 3 + c] / maxQuantizedValue ); }
        colorUnitTo256( inYUV, inYUV_256 );
        yuvBt709ToRgb_256( inYUV, inRGB_256 );

        // vectorized by compiler
        for ( glm::vec3::length_type c = 0; c < 3; ++c ) { output.colors[i * 3 + c] = inRGB_256[c]; }
      } else {
        for ( glm::vec3::length_type c = 0; c < 3; ++c ) {
          output.colors[i * 3 + c] = ( input.colors[i * 3 + c] * range / maxQuantizedValue ) + minBox[c];
        }
      }
    }
  }
}
