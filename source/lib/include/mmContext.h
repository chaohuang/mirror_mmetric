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

#ifndef _MM_CONTEXT_H_
#define _MM_CONTEXT_H_

#include <string>
#include <map>
#include <iostream>

class Context {
 public:
  Context() : _frame( 0 ), _firstFrame( 0 ), _lastFrame( 0 ) {}

  bool setFrame( uint32_t frame ) {
    if ( frame < _firstFrame || frame > _lastFrame ) { return false; }
    _frame = frame;
    return true;
  }
  uint32_t getFrame( void ) { return _frame; }

  // first and last frame are included
  bool setFrameRange( uint32_t first, uint32_t last ) {
    if ( first > last ) return false;
    _firstFrame = first;
    _lastFrame  = last;
    return true;
  }

  uint32_t getFirstFrame( void ) { return _firstFrame; }
  uint32_t getLastFrame( void ) { return _lastFrame; }
  uint32_t getFrameCount( void ) { return _lastFrame - _firstFrame + 1; }

 private:
  uint32_t _frame;  // current frame
  uint32_t _firstFrame;
  uint32_t _lastFrame;
};

#endif
