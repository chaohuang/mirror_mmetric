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

#ifndef _MM_CMD_ANALYSE_H_
#define _MM_CMD_ANALYSE_H_

#include <limits>

// internal headers
#include "mmCommand.h"
#include "mmModel.h"

class CmdAnalyse : Command {
 private:
  // the context for frame access
  Context* _context;
  // the command options
  std::string _inputModelFilename;
  std::string _inputTextureFilename;
  std::string _outputCsvFilename;
  std::string _outputVarFilename;
  // count statistics results array of <frame, nbface, nbvert, nbcol, nbnorm, nbuv>
  std::vector<std::tuple<uint32_t, double, double, double, double, double> > _counts;
  // renge results
  glm::vec3 _minPos = {std::numeric_limits<float>::max(), std::numeric_limits<float>::max(),
                       std::numeric_limits<float>::max()};
  glm::vec3 _maxPos = {std::numeric_limits<float>::min(), std::numeric_limits<float>::min(),
                       std::numeric_limits<float>::min()};
  glm::vec3 _minCol = {255, 255, 255};
  glm::vec3 _maxCol = {0, 0, 0};
  glm::vec3 _minNrm = {std::numeric_limits<float>::max(), std::numeric_limits<float>::max(),
                       std::numeric_limits<float>::max()};
  glm::vec3 _maxNrm = {std::numeric_limits<float>::min(), std::numeric_limits<float>::min(),
                       std::numeric_limits<float>::min()};
  glm::vec2 _minUv  = {std::numeric_limits<float>::max(), std::numeric_limits<float>::max()};
  glm::vec2 _maxUv  = {std::numeric_limits<float>::min(), std::numeric_limits<float>::min()};

 public:
  CmdAnalyse(){};

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
