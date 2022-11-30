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

#ifndef _MM_DEQUANTIZE_H_
#define _MM_DEQUANTIZE_H_

// internal headers
#include "mmModel.h"

namespace mm {

class Dequantize {
 public:
  Dequantize(){};

  static void dequantize( const Model&     input,
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
                          const bool       verbose = true );
};

}  // namespace mm

#endif
