// ************* COPYRIGHT AND CONFIDENTIALITY INFORMATION *********
// Copyright © 20XX InterDigital All Rights Reserved
// This program contains proprietary information which is a trade secret/business
// secret of InterDigital R&D france is protected, even if unpublished, under 
// applicable Copyright laws (including French droit d’auteur) and/or may be 
// subject to one or more patent(s).
// Recipient is to retain this program in confidence and is not permitted to use 
// or make copies thereof other than as permitted in a written agreement with 
// InterDigital unless otherwise expressly allowed by applicable laws or by 
// InterDigital under express agreement.
//
// Author: jean-eudes.marvie@interdigital.com
// *****************************************************************

#ifndef _MM_IO_H_
#define _MM_IO_H_

#include <string>
#include "mmModel.h"
#include "mmImage.h"

class IO {

	IO() {}

public:

	// name can be filename or "ID:xxxx"
	// return NULL in case of error
	static Model* loadModel(std::string name) {
		std::map<std::string, Model*>::iterator it = IO::_models.find(name);
		if (it == IO::_models.end()) {
			if (name.substr(0, 3) == "ID:") {
				std::cout << "Error: model with id " << name << "not defined" << std::endl;
				return NULL;
			}
			else { // we try to load the model
				Model* model = new Model();
				if (!IO::_loadModel(name, *model)) {
					delete model;
					return NULL;
				}
				else {
					IO::_models[name] = model;
					return model;
				}
			}
		}
		return it->second;
	};

	static bool saveModel(std::string name, Model* model) {
		std::map<std::string, Model*>::iterator it = IO::_models.find(name);
		if (it != IO::_models.end()) {
			std::cout << "Warning: model with id " << name << "already defined, overwriting" << std::endl;
			delete it->second;
			it->second = model;
		}
		else {
			IO::_models[name] = model;
		}
		// save to file if not an id
		if (name.substr(0, 3) != "ID:") {
			return IO::_saveModel(name, *model);
		}
		return true;
	}

	// name can be filename or "ID:xxxx"
	// return NULL in case of error
	static Image* loadImage(std::string name) {
		std::map<std::string, Image*>::iterator it = IO::_images.find(name);
		if (it == IO::_images.end()) {
			if (name.substr(0, 3) == "ID:") {
				std::cout << "Error: image with id " << name << "not defined" << std::endl;
				return NULL;
			}
			else { // we try to load the model
				Image* image = new Image();
				if (!IO::_loadImage(name, *image)) {
					delete image;
					return NULL;
				}
				else {
					IO::_images[name] = image;
					return image;
				}
			}
		}
		return it->second;
	};

	/*
	static bool saveImage(std::string name, Image* image) {
		std::map<std::string, Image*>::iterator it = IO::_images.find(name);
		if (it == IO::_images.end()) {
			std::cout << "Warning: image with id " << name << "already defined, overwriting" << std::endl;
			delete it->second;
			it->second = image;
		}
		else {
			IO::_images[name] = image;
		}
		// save to file if not an id
		if (name.substr(0, 3) != "ID:") {
			IO::_saveImage(name, *image);
		}
	}*/

private:

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

};

#endif