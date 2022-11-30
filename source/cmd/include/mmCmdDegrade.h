// ************* COPYRIGHT AND CONFIDENTIALITY INFORMATION *********
// Copyright � 20XX InterDigital All Rights Reserved
// This program contains proprietary information which is a trade secret/business
// secret of InterDigital R&D france is protected, even if unpublished, under
// applicable Copyright laws (including French droit d�auteur) and/or may be
// subject to one or more patent(s).
// Recipient is to retain this program in confidence and is not permitted to use
// or make copies thereof other than as permitted in a written agreement with
// InterDigital unless otherwise expressly allowed by applicable laws or by
// InterDigital under express agreement.
//
// Author: jean-eudes.marvie@interdigital.com
// *****************************************************************

#ifndef _MM_CMD_DEGRADE_H_
#define _MM_CMD_DEGRADE_H_

// internal headers
#include "mmCommand.h"
#include "mmModel.h"
#include "mmImage.h"

class CmdDegrade : Command {
 private:
  // the command options
  std::string _inputModelFilename;
  std::string _outputModelFilename;
  std::string _mode    = "delface";
  size_t      _nthFace = 50;  // skip every nth face
  size_t      _nbFaces = 0;   // if nthFace==0, skip number of faces

 public:
  CmdDegrade(){};

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
  size_t delNthFace( const mm::Model& input, size_t nthFace, mm::Model& output );
  size_t delNbFaces( const mm::Model& input, size_t nthFace, mm::Model& output );
};

#endif