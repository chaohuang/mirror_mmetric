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

#ifndef _MM_IO_H_
#define _MM_IO_H_

#include <map>
#include <string>

#include "mmModel.h"
#include "mmImage.h"
#include "mmContext.h"

class IO {

	IO() {}

public:
	
	// stores the context to sol
	static void setContext(Context* context);

	// converts a string containing "%nd" with n an integer into a string 
	// containing the frame number with n digits
	// e.g. filename00%3d.png filename00156.png if frame is 156
	static std::string resolveName(const uint32_t frame, const std::string& input);

	// name can be filename or "ID:xxxx"
	// return NULL in case of error
	static Model* loadModel(std::string templateName);

	static bool saveModel(std::string templateName, Model* model);

	// load image files and images from videos
	// name can be filename or "ID:xxxx"
	// return NULL in case of error
	static Image* loadImage(std::string templateName);

	/*
	static bool saveImage(std::string name, Image* image);*/

	// free all the models and images, and reset cache.
	static void purge(void);

private:

	// access to context for frame name resolution
	static Context* _context;

	// model store
	static std::map<std::string, Model*> _models;
	// image store
	static std::map<std::string, Image*> _images;

private: 

	// Automatic choice on extension
	static bool _loadModel(std::string filename, Model& output);
	static bool _saveModel(std::string filename, const Model& input);

	// OBJ
	static bool _loadObj(std::string filename, Model& output);
	static bool _saveObj(std::string filename, const Model& input);

	// PLY
	static bool _loadPly(std::string filename, Model& output);
	static bool _savePly(std::string filename, const Model& input);

	// Images
	static bool _loadImage(std::string filename, Image& output);
	static bool _loadImageFromVideo(std::string filename, Image& output);


};

#endif