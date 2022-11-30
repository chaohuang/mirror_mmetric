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

#ifndef _MM_CMD_SAMPLE_H_
#define _MM_CMD_SAMPLE_H_

// internal headers
#include "mmCommand.h"
#include "mmModel.h"
#include "mmImage.h"

class CmdSample : Command {
 private:
  // the context for frame access
  Context* _context;
  // the command options
  std::string inputModelFilename;
  std::string inputTextureFilename;
  std::string outputModelFilename;
  std::string _outputCsvFilename;
  bool        hideProgress = false;
  // the type of processing
  std::string mode = "face";
  // Face options
  float thickness = 0.0;
  // Grid options
  int         _gridSize      = 1024;
  bool        _useNormal     = false;
  bool        _useFixedPoint = false;
  std::string _minPosStr;
  std::string _maxPosStr;
  glm::vec3   _minPos = {0.0F, 0.0F, 0.0F};
  glm::vec3   _maxPos = {0.0F, 0.0F, 0.0F};
  // Face, Grid and sdiv options
  bool bilinear = false;
  // Face subdiv options
  float areaThreshold = 1.0F;
  bool  mapThreshold  = false;
  // Edge subdiv options (0.0 mean use resolution)
  float lengthThreshold = 0.0F;
  // Edge and Face options
  size_t _resolution = 1024;
  // sample count constrained sampling
  size_t _nbSamplesMin  = 0;
  size_t _nbSamplesMax  = 0;
  size_t _maxIterations = 10;
  // Prnd options
  size_t _nbSamples = 2000000;

 public:
  CmdSample(){};

  // Description of the command
  static const char* name;
  static const char* brief;
  // command creator
  static Command* create();

  // the command main program
  virtual bool initialize( Context* ctx, std::string app, int argc, char* argv[] );
  virtual bool process( uint32_t frame );
  virtual bool finalize() { return true; }
};

#endif
