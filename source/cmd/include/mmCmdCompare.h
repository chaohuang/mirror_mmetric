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

#ifndef _MM_CMD_COMPARE_H_
#define _MM_CMD_COMPARE_H_

// internal headers
#include "mmCommand.h"
#include "mmModel.h"
#include "mmCompare.h"

class CmdCompare : Command {
 private:
  // the context for frame access
  Context* _context;
  // the command options
  std::string _inputModelAFilename, _inputModelBFilename;
  std::string _inputTextureAFilename, _inputTextureBFilename;
  std::string _outputModelAFilename, _outputModelBFilename;
  std::string _outputCsvFilename;
  // the type of processing
  std::string _mode = "equ";
  // Equ options
  float _equEpsilon     = 0;
  bool  _equEarlyReturn = true;
  bool  _equUnoriented  = false;
  // Topo options
  std::string _topoFaceMapFilename;
  std::string _topoVertexMapFilename;
  // Pcc options
  pcc_quality::commandPar _pccParams;
  // PCQM options
  double _pcqmRadiusCurvature    = 0.001;
  int    _pcqmThresholdKnnSearch = 20;
  double _pcqmRadiusFactor       = 2.0;

  // Raster options
  unsigned int _ibsmResolution        = 2048;
  unsigned int _ibsmCameraCount       = 16;
  std::string  _ibsmCameraRotation    = "0.0 0.0 0.0";
  glm::vec3    _ibsmCamRotParams      = {0.0F, 0.0F, 0.0F};
  std::string  _ibsmRenderer          = "sw_raster";
  std::string  _ibsmOutputPrefix      = "";
  bool         _ibsmDisableReordering = false;
  bool         _ibsmDisableCulling    = false;

  // Compare
  mm::Compare _compare;

 public:
  CmdCompare() {
    _pccParams.singlePass      = false;
    _pccParams.hausdorff       = false;
    _pccParams.bColor          = true;
    _pccParams.bLidar          = false;  // allways false, no option
    _pccParams.resolution      = 0.0;    // auto
    _pccParams.neighborsProc   = 1;
    _pccParams.dropDuplicates  = 2;
    _pccParams.bAverageNormals = true;

    // Modification of D2 metric is enabled for mmetric
    _pccParams.normalCalcModificationEnable = true;
  };

  // Descriptions of the command
  static const char* name;
  static const char* brief;

  // command creator
  static Command* create();

  // the command main program
  virtual bool initialize( Context* ctx, std::string app, int argc, char* argv[] );
  virtual bool process( uint32_t frame );  
  virtual bool finalize();
  
};

#endif
