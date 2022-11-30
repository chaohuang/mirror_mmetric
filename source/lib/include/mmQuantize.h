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

#ifndef _MM_QUANTIZE_H_
#define _MM_QUANTIZE_H_

// internal headers
#include "mmModel.h"
#include <limits>

namespace mm {

class Quantize {
 public:
  Quantize(){};

  // quantizes the model using parameters and range. If a range is not set (i.e. min == max) then a range is
  // computed internally. If quantization parameter is < 7, the related attribute is not quantized.
  static void quantize(const Model&       input,
                       Model&             output,
                       const uint32_t     qp,
                       const uint32_t     qt,
                       const uint32_t     qn,
                       const uint32_t     qc,
                       const std::string& outputVarFilename,
                       bool               useFixedPoint,
                       bool               colorSpaceConversion,
                       glm::vec3&         minPos,
                       glm::vec3&         maxPos,
                       glm::vec2&         minUv,
                       glm::vec2&         maxUv,
                       glm::vec3&         minNrm,
                       glm::vec3&         maxNrm,
                       glm::vec3&         minCol,
                       glm::vec3&         maxCol,
                       const bool         verbose = true);
};

}  // namespace mm

#endif
