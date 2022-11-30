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

#ifndef _MM_SAMPLE_H_
#define _MM_SAMPLE_H_

// internal headers
#include "mmModel.h"
#include "mmImage.h"

namespace mm {

class Sample {
 public:
  Sample(){};

  // sample the mesh on a face basis
  static void meshToPcFace( const Model& input,
                            Model&       output,
                            const Image& tex_map,
                            size_t       resolution,
                            float        thickness,
                            bool         bilinear,
                            bool         logProgress );

  // sample the mesh on a face basis
  // system will search the resolution according to the nbSamplesMin and nbSamplesMax parameters
  // costly method, shall be used only for calibration
  static void meshToPcFace( const Model& input,
                            Model&       output,
                            const Image& tex_map,
                            size_t       nbSamplesMin,
                            size_t       nbSamplesMax,
                            size_t       maxIterations,
                            float        thickness,
                            bool         bilinear,
                            bool         logProgress,
                            size_t&      computedResolution );

  // will sample the mesh on a grid basis of resolution gridRes
  static void meshToPcGrid( const Model& input,
                            Model&       output,
                            const Image& tex_map,
                            size_t       gridSize,
                            bool         bilinear,
                            bool         logProgress,
                            bool         useNormal,
                            bool         useFixedPoint,
                            glm::vec3&   minPos,
                            glm::vec3&   maxPos,
                            const bool   verbose = true );

  // will sample the mesh on a grid basis of resolution gridRes, result will be generated as float or integer
  // system will search the resolution according to the nbSamplesMin and nbSamplesMax parameters
  // costly method, shall be used only for calibration
  static void meshToPcGrid( const Model& input,
                            Model&       output,
                            const Image& tex_map,
                            size_t       nbSamplesMin,
                            size_t       nbSamplesMax,
                            size_t       maxIterations,
                            bool         bilinear,
                            bool         logProgress,
                            bool         useNormal,
                            bool         useFixedPoint,
                            glm::vec3&   minPos,
                            glm::vec3&   maxPos,
                            size_t&      computedResolution );

  // revert sampling, guided by texture map
  static void meshToPcMap( const Model& input, Model& output, const Image& tex_map, bool logProgress );

  // triangle dubdivision based, area stop criterion
  static void meshToPcDiv( const Model& input,
                           Model&       output,
                           const Image& tex_map,
                           float        areaThreshold,
                           bool         mapThreshold,
                           bool         bilinear,
                           bool         logProgress );

  // triangle dubdivision based, area stop criterion
  // system will search the resolution according to the nbSamplesMin and nbSamplesMax parameters
  // costly method, shall be used only for calibration
  static void meshToPcDiv( const Model& input,
                           Model&       output,
                           const Image& tex_map,
                           size_t       nbSamplesMin,
                           size_t       nbSamplesMax,
                           size_t       maxIterations,
                           bool         bilinear,
                           bool         logProgress,
                           float&       computedThres );

  // triangle dubdivision based, edge stop criterion
  static void meshToPcDivEdge( const Model& input,
                               Model&       output,
                               const Image& tex_map,
                               float        lengthThreshold,
                               size_t       resolution,
                               bool         bilinear,
                               bool         logProgress,
                               float&       computedThres );

  // triangle dubdivision based, edge stop criterion
  // system will search the resolution according to the nbSamplesMin and nbSamplesMax parameters
  // costly method, shall be used only for calibration
  static void meshToPcDivEdge( const Model& input,
                               Model&       output,
                               const Image& tex_map,
                               size_t       nbSamplesMin,
                               size_t       nbSamplesMax,
                               size_t       maxIterations,
                               bool         bilinear,
                               bool         logProgress,
                               float&       computedThres );

  // pseudo random sampling with point targetPointCount stop criterion
  static void meshToPcPrnd( const Model& input,
                            Model&       output,
                            const Image& tex_map,
                            size_t       targetPointCount,
                            bool         bilinear,
                            bool         logProgress );
};

}  // namespace mm

#endif
