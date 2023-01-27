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

#ifndef _MM_CMD_QUANTIZE_H_
#define _MM_CMD_QUANTIZE_H_

// internal headers
#include "mmCommand.h"
#include "mmModel.h"
#include <limits>

class CmdQuantize : Command {
 public:
  CmdQuantize(){};

  // Description of the command
  static const char* name;
  static const char* brief;
  // command creator
  static Command* create();

  // the command main program
  virtual bool initialize( Context* ctx, std::string app, int argc, char* argv[] );
  virtual bool process( uint32_t frame );
  virtual bool finalize() { return true; };

 private:
  // Command parameters
  std::string _inputModelFilename;
  std::string _outputModelFilename;
  std::string _outputVarFilename;
  // Quantization parameters
  uint32_t _qp         = 12;  // geometry
  uint32_t _qt         = 12;  // UV coordinates
  uint32_t _qn         = 12;  // normals
  uint32_t _qc         = 8;   // colors
  bool     _dequantize = false;
  //
  bool _computeBboxPos = false;
  bool _computeBboxUv  = false;
  bool _computeBboxNrm = false;
  bool _computeBboxCol = false;
  bool _useFixedPoint  = false;
  // min max vectors
  std::string _minPosStr;
  std::string _maxPosStr;
  glm::vec3   _minPos = {0.0F, 0.0F, 0.0F};
  glm::vec3   _maxPos = {0.0F, 0.0F, 0.0F};
  std::string _minUvStr;
  std::string _maxUvStr;
  glm::vec2   _minUv = {0.0F, 0.0F};
  glm::vec2   _maxUv = {0.0F, 0.0F};
  std::string _minNrmStr;
  std::string _maxNrmStr;
  glm::vec3   _minNrm = {0.0F, 0.0F, 0.0F};
  glm::vec3   _maxNrm = {0.0F, 0.0F, 0.0F};
  std::string _minColStr;
  std::string _maxColStr;
  glm::vec3   _minCol = {0.0F, 0.0F, 0.0F};
  glm::vec3   _maxCol = {0.0F, 0.0F, 0.0F};
  // color space conversion
  bool _colorSpaceConversion = false;
};

#endif