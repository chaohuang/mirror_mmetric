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

#ifndef _MM_COMPARE_H_
#define _MM_COMPARE_H_

#include "mmModel.h"
#include "mmContext.h"
#include "mmRendererHw.h"
#include "mmRendererSw.h"
// MPEG PCC metric
#include "dmetric/source/pcc_distortion.hpp"

namespace mm {

class Compare {
 public:
  struct IbsmResults {
    double rgbMSE[4]  = { 0, 0, 0, 0 };  // [R mse, G mse, B mse, components mean]
    double rgbPSNR[4] = { 0, 0, 0, 0 };  // idem but with psnr
    double yuvMSE[4]  = { 0, 0, 0, 0 };  // [Y mse, U mse, V mse, 6/1/1 mean ]
    double yuvPSNR[4] = { 0, 0, 0, 0 };  // idem but with psnr
    double depthMSE   = 0;
    double depthPSNR  = 0;
    double boxRatio;  // ( dis box size / ref box size ) * 100.0
		double unmatchedPixelPercentage = 0;	// ( unmatchedPixelsSum /  maskSizeSum ) * 100.0
  };

  Compare(); 
  ~Compare();  

 private:
  // Pcc results array of <frame, result>
  std::vector<std::pair<uint32_t, pcc_quality::qMetric> > _pccResults;
  // PCQM results array of <frame, pcqm, pcqm-psnr>
  std::vector<std::tuple<uint32_t, double, double> > _pcqmResults;
  // Raster results array of <frame, result>
  std::vector<std::pair<uint32_t, IbsmResults> > _ibsmResults;

  // Renderers for the ibsm metric
  mm::RendererSw _swRenderer;             // the Software renderer
  mm::RendererHw _hwRenderer;             // the Hardware renderer
  bool           _hwRendererInitialized;  // the Hardware renderer is initilized

 public:
  auto& getPccResults() { return _pccResults.back(); };
  auto& getPcqmResults() { return _pcqmResults.back(); };
  auto& getIbsmResults() { return _ibsmResults.back(); };

  size_t size() {
    return (std::max)(_pccResults.size(),
                      (std::max)(_pcqmResults.size(), _pcqmResults.size()));
  }
  std::vector<double> getFinalPccResults();
  std::vector<double> getFinalPcqmResults();
  std::vector<double> getFinalIbsmResults();
  std::vector<double> getPccResults( const size_t index );
  std::vector<double> getPcqmResults( const size_t index );
  std::vector<double> getIbsmResults( const size_t index );

  // compare two meshes for equality (using mem comp if epsilon = 0)
  // if epsilon = 0, return 0 on success and 1 on difference
  // if epsilon > 0, return 0 on success and nb diff on difference if sizes are equal, 1 otherwise
  int equ( const mm::Model& modelA,
           const mm::Model& modelB,
           const mm::Image& mapA,
           const mm::Image& mapB,
           float            epsilon,
           bool             earlyReturn,
           bool             unoriented,
           mm::Model&       outputA,
           mm::Model&       outputB );

  // compare two meshes topology for equivalence up to face index shift
  // check topology will use a bijective face map, associating output triangles to input triangles:
  // - faceMap file shall contain the association dest face index -> orig face index for each face, one face per line
  // - %d in filename is to be resolved before invocation
  // check topology will use a bijective vartex map, associating output vertices to input vertices:
  // - vertexMap file shall contain the association dest vertex index -> orig vertex index for each vertex, one vertex
  // per line
  // - %d in filename is to be resolved before invocation
  // the function validates the following points:
  // - Test if number of triangles of output matches input number of triangles
  // - Test if the proposed association tables for face and vertex are bijective
  // - Test if each output triangle respects the orientation of its associated input triangle
  int topo( const mm::Model&   modelA,
            const mm::Model&   modelB,
            const std::string& faceMapFilenane   = "",
            const std::string& vertexMapFilenane = "" );

  // compare two meshes using MPEG pcc_distortion metric
  int pcc( const mm::Model&         modelA,
           const mm::Model&         modelB,
           const mm::Image&         mapA,
           const mm::Image&         mapB,
           pcc_quality::commandPar& params,
           mm::Model&               outputA,
           mm::Model&               outputB,
           const bool               verbose = true);

  // collect statics over sequence and compute results
  void pccFinalize( void );

  // compare two meshes using PCQM metric
  int pcqm( const mm::Model& modelA,
            const mm::Model& modelB,
            const mm::Image& mapA,
            const mm::Image& mapB,
            const double     radiusCurvature,
            const int        thresholdKnnSearch,
            const double     radiusFactor,
            mm::Model&       outputA,
            mm::Model&       outputB,
            const bool       verbose = true );

  // collect statics over sequence and compute results
  void pcqmFinalize( void );

  // compare two meshes using rasterization
  int ibsm( const mm::Model&   modelA,
            const mm::Model&   modelB,
            const mm::Image&   mapA,
            const mm::Image&   mapB,
            const bool         disableReordering,
            const uint32_t     resolution,
            const uint32_t     cameraCount,
            const glm::vec3&   camRotParams,
            const std::string& renderer,
            const std::string& outputPrefix,
            const bool         disableCulling,
            mm::Model&         outputA,
            mm::Model&         outputB,
            const bool         verbose = true );

  // collect statics over sequence and compute results
  void ibsmFinalize( void );
};

}  // namespace mm

#endif
