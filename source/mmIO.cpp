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
// Author: loadObj/saveObj methods based on original code from Owlii
// *****************************************************************

// remove warning when using sprintf on MSVC
#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <iostream>
#include <fstream>
#include <unordered_map>
#include <time.h>
#include <cmath>
// ply loader
#define TINYPLY_IMPLEMENTATION
#include "tinyply.h"
// images
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
// mathematics
#include <glm/vec3.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include "mmIO.h"

//
Context* IO::_context = NULL;
// create the stores
std::map<std::string, Model*> IO::_models;
std::map<std::string, Image*> IO::_images;

//
void  IO::setContext(Context* context) {
	_context = context;
}

//
std::string  IO::resolveName(const uint32_t frame, const std::string& input) {
	std::string output;
	if (input.find("%") != std::string::npos) {
		char buffer[4092];
		auto n = sprintf(buffer, input.c_str(), frame);
		output = buffer;
		std::cout << output << std::endl;
	}
	else {
		output = input;
	}
	return output;
}

//
Model* IO::loadModel(std::string templateName) {
	std::string name = resolveName(_context->getFrame(), templateName);
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

//
bool IO::saveModel(std::string templateName, Model* model) {
	std::string name = resolveName(_context->getFrame(), templateName);
	std::map<std::string, Model*>::iterator it = IO::_models.find(name);
	if (it != IO::_models.end()) {
		std::cout << "Warning: model with id " << name << " already defined, overwriting" << std::endl;
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

//
Image* IO::loadImage(std::string templateName) {
	
	// The IO store is purged for each new frame. 
	// So in case of video file without %d template we just use the filename (unchanged by resolveName).
	std::string name = resolveName(_context->getFrame(), templateName);
	std::map<std::string, Image*>::iterator it = IO::_images.find(name);
	
	// use image/frame from store
	if (it != IO::_images.end()) {
		return it->second;
	}
	
	// not found in store but name is an ID => error
	if (name.substr(0, 3) == "ID:") {
		std::cout << "Error: image with id " << name << "not defined" << std::endl;
		return NULL;
	}

	// else try to load the image/frame
	Image* image = new Image();

	std::string ext = templateName.substr(templateName.find_last_of("."));
	std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) { return std::tolower(c); });

	if (ext == ".yuv" || ext == ".rgb") {
		// try to load as video
		if (!IO::_loadImageFromVideo(name, *image)) {
			delete image;
			return NULL;
		}
	}
	else {
		// try to load as image
		if (!IO::_loadImage(name, *image)) {
			delete image;
			return NULL;
		}
	}
	// add to the store
	IO::_images[name] = image;
	return image;	
};

/*
bool  IO::saveImage(std::string name, Image* image) {
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

// 
void IO::purge(void) {
	// free all the texture maps
	std::map<std::string, Image*>::iterator imageIt = IO::_images.begin();
	for (; imageIt != _images.end(); ++imageIt) {
		delete imageIt->second;
	}
	_images.clear();

	// free all the models
	std::map<std::string, Model*>::iterator modelIt = IO::_models.begin();
	for (; modelIt != _models.end(); ++modelIt) {
		delete modelIt->second;
	}
	_models.clear();
}

///////////////////////////
// Private methods

bool IO::_loadModel(std::string filename, Model& output) {

	bool success = true;

	// sanity check
	if (filename.size() < 5) {
		std::cout << "Error, invalid mesh file name " << filename << std::endl;
		return false;
	}

	// get extension
	std::string ext = filename.substr(filename.size() - 3, 3);
	std::for_each(ext.begin(), ext.end(), [](char& c) { c = ::tolower(c); });

	// do the job
	if (ext == "ply") {
		std::cout << "Loading file: " << filename << std::endl;
		auto t1 = clock();
		success = IO::_loadPly(filename, output); // TODO handle read error
		if (success) {
			auto t2 = clock();
			std::cout << "Time on loading: " << ((float)(t2 - t1)) / CLOCKS_PER_SEC << " sec." << std::endl;
		}
	}
	else if (ext == "obj") {
		std::cout << "Loading file: " << filename << std::endl;
		auto t1 = clock();
		success = IO::_loadObj(filename, output); // TODO handle read error
		if (success) {
			auto t2 = clock();
			std::cout << "Time on loading: " << ((float)(t2 - t1)) / CLOCKS_PER_SEC << " sec." << std::endl;
		}
	}
	else {
		std::cout << "Error, invalid mesh file extension (not in obj, ply)" << std::endl;
		return false;
	}

	if (success) {
		// print stats
		std::cout << "Input model: "   << filename << std::endl;
		std::cout << "  Vertices: "    << output.vertices.size() / 3 << std::endl;
		std::cout << "  UVs: "         << output.uvcoords.size() / 2 << std::endl;
		std::cout << "  Colors: "      << output.colors.size() / 3 << std::endl;
		std::cout << "  Normals: "     << output.normals.size() / 3 << std::endl;
		std::cout << "  Triangles: "   << output.triangles.size() / 3 << std::endl;
		std::cout << "  Trianglesuv: " << output.trianglesuv.size() / 3 << std::endl;
	}

	return success;

}

bool IO::_saveModel(std::string filename, const Model& input) {

	// sanity check
	if (filename.size() < 5) {
		std::cout << "Error, invalid mesh file name " << filename << std::endl;
		return false;
	}

	// check output file extension
	std::string out_ext = filename.substr(filename.size() - 3, 3);
	std::for_each(out_ext.begin(), out_ext.end(), [](char& c) { c = ::tolower(c); });

	// write output
	if (out_ext == "ply") {
		std::cout << "Saving file: " << filename << std::endl;
		auto t1 = clock();
		auto err = IO::_savePly(filename, input);
		if (!err) {
			auto t2 = clock();
			std::cout << "Time on saving: " << ((float)(t2 - t1)) / CLOCKS_PER_SEC << " sec." << std::endl;
		}
		return err;
	}
	else if (out_ext == "obj") {
		std::cout << "Saving file: " << filename << std::endl;
		auto t1 = clock();
		auto err = IO::_saveObj(filename, input);
		if (!err) {
			auto t2 = clock();
			std::cout << "Time on saving: " << ((float)(t2 - t1)) / CLOCKS_PER_SEC << " sec." << std::endl;
		}
		return err;
	}
	else {
		std::cout << "Error: invalid mesh file extension (not in obj, ply)" << std::endl;
		return false;
	}

	// success
	return true;
}

bool IO::_loadObj(std::string filename, Model& output) {

	std::ifstream fin;
	// use a big 4MB buffer to accelerate reads
	char* buf = new char[4 * 1024 * 1024 + 1]; 
	fin.rdbuf()->pubsetbuf(buf, 4 * 1024 * 1024 + 1);
	fin.open(filename.c_str(), std::ios::in);
	if (!fin)
	{
		std::cerr << "Error: can't open file " << filename << std::endl;
		delete[] buf;
		return false;
	}
	int temp_index;
	float temp_pos, temp_uv, temp_normal, temp_col;
	std::string temp_flag, temp_str;
	std::string line;
	std::getline(fin, line);
	while (fin) {
		std::istringstream in(line);
		temp_flag = "";
		in >> temp_flag;  // temp_flag: the first word in the line
		if (temp_flag.compare(std::string("mtllib")) == 0) {
			output.header = std::string(line.c_str());
		}
		else if (temp_flag.compare(std::string("v")) == 0) {
			// parse the position
			for (int i = 0; i < 3; i++) {
				in >> temp_pos;
				output.vertices.push_back(temp_pos);
			}
			// parse the color if any (re map 0.0-1.0 to 0-255 internal color format)
			while (in >> temp_col) {
				output.colors.push_back(std::roundf(temp_col * 255));
			}
		}
		else if (temp_flag.compare(std::string("vn")) == 0) {
			for (int i = 0; i < 3; i++) {
				in >> temp_normal;
				output.normals.push_back(temp_normal);
			}
		}
		else if (temp_flag.compare(std::string("vt")) == 0) {
			for (int i = 0; i < 2; i++) {
				in >> temp_uv;
				output.uvcoords.push_back(temp_uv);
			}
		}
		else if (temp_flag.compare(std::string("f")) == 0) {
			for (int i = 0; i < 3; i++) {
				in >> temp_str;
				size_t found;

				// parsing of texture coord indexes
				// TODO parsing of normals indexes and reindex
				found = temp_str.find_first_of("/");
				if (found != std::string::npos) {
					temp_index = atoi(temp_str.substr(0, found).c_str()) - 1;
					int uv_index = atoi(temp_str.substr(found + 1, temp_str.size() - found).c_str()) - 1;
					output.trianglesuv.push_back(uv_index);
				}
				else
					temp_index = atoi(temp_str.c_str()) - 1;
				output.triangles.push_back(temp_index);
			}
		}
		std::getline(fin, line);
	}
	fin.close();
	delete[] buf;

	if (output.normals.size() != 0 && output.normals.size() != output.vertices.size()) {
		std::cout << "Warning: obj read, normals with separate index table are not yet supported. Skipping normals." << std::endl;
		output.normals.clear();
	}

	return true;
}

bool IO::_saveObj(std::string filename, const Model& input) {

	std::ofstream fout;
	// use a big 4MB buffer to accelerate writes
	char* buf = new char[4 * 1024 * 1024 + 1]; 
	fout.rdbuf()->pubsetbuf(buf, 4 * 1024 * 1024 + 1);
	fout.open(filename.c_str(), std::ios::out); 
	if (!fout)
	{
		std::cerr << "Error: can't open file " << filename << std::endl;
		delete[] buf;
		return false;
	}
	// this is mandatory to print floats with full precision
	fout.precision(std::numeric_limits< float >::max_digits10);

	fout << input.header << std::endl;
	for (int i = 0; i < input.vertices.size() / 3; i++) {
		fout << "v " <<
			input.vertices[i * 3 + 0] << " " <<
			input.vertices[i * 3 + 1] << " " <<
			input.vertices[i * 3 + 2] ;
		if (input.colors.size() == input.vertices.size()) {
			fout << " " <<
				input.colors[i * 3 + 0] / 255 << " " <<
				input.colors[i * 3 + 1] / 255 << " " <<
				input.colors[i * 3 + 2] / 255 << std::endl;
		}
		else {
			fout << std::endl;
		}
	}
	for (int i = 0; i < input.normals.size() / 3; i++) {
		fout << "vn " <<
			input.normals[i * 3 + 0] << " " <<
			input.normals[i * 3 + 1] << " " <<
			input.normals[i * 3 + 2] << std::endl;
	}
	for (int i = 0; i < input.uvcoords.size() / 2; i++) {
		fout << "vt " <<
			input.uvcoords[i * 2 + 0] << " " <<
			input.uvcoords[i * 2 + 1] << std::endl;
	}
	fout << "usemtl material0000" << std::endl;
	for (int i = 0; i < input.triangles.size() / 3; i++) {

		if (input.trianglesuv.size() == input.triangles.size() ){
			fout << "f " <<
				input.triangles[i * 3 + 0] + 1 << "/" << input.trianglesuv[i * 3 + 0] + 1 << " " <<
				input.triangles[i * 3 + 1] + 1 << "/" << input.trianglesuv[i * 3 + 1] + 1 << " " <<
				input.triangles[i * 3 + 2] + 1 << "/" << input.trianglesuv[i * 3 + 2] + 1 << std::endl;
		}
		else {
			fout << "f " <<
				input.triangles[i * 3 + 0] + 1 << " " <<
				input.triangles[i * 3 + 1] + 1 << " " <<
				input.triangles[i * 3 + 2] + 1 << std::endl;
		}
	}
	fout.close();
	delete[] buf;
	return true;
}


bool IO::_loadPly(std::string filename, Model& output)
{
	std::unique_ptr<std::istream> file_stream;
	file_stream.reset(new std::ifstream(filename.c_str(), std::ios::binary));
	tinyply::PlyFile file;
	file.parse_header(*file_stream);

	std::shared_ptr<tinyply::PlyData> _vertices, _normals, _colors, _colorsRGBA, _texcoords, _faces, _tripstrip;

	// The header information can be used to programmatically extract properties on elements
	// known to exist in the header prior to reading the data. For brevity of this sample, properties 
	// like vertex position are hard-coded: 
	try { _vertices = file.request_properties_from_element("vertex", { "x", "y", "z" }); }
	catch (const std::exception& e) { std::cerr << "skipping: " << e.what() << std::endl; }

	try { _normals = file.request_properties_from_element("vertex", { "nx", "ny", "nz" }); }
	catch (const std::exception& e) { std::cerr << "skipping: " << e.what() << std::endl; }

	try { _colorsRGBA = file.request_properties_from_element("vertex", { "red", "green", "blue", "alpha" }); }
	catch (const std::exception& e) { std::cerr << "skipping: " << e.what() << std::endl; }

	try { _colorsRGBA = file.request_properties_from_element("vertex", { "r", "g", "b", "a" }); }
	catch (const std::exception& e) { std::cerr << "skipping: " << e.what() << std::endl; }

	try { _colors = file.request_properties_from_element("vertex", { "r", "g", "b" }); }
	catch (const std::exception& e) { std::cerr << "skipping: " << e.what() << std::endl; }

	try { _colors = file.request_properties_from_element("vertex", { "red", "green", "blue" }); }
	catch (const std::exception& e) { std::cerr << "skipping: " << e.what() << std::endl; }

	try { _texcoords = file.request_properties_from_element("vertex", { "u", "v" }); }
	catch (const std::exception& e) { std::cerr << "skipping: " << e.what() << std::endl; }

	// Providing a list size hint (the last argument) is a 2x performance improvement. If you have 
	// arbitrary ply files, it is best to leave this 0. 
	try { _faces = file.request_properties_from_element("face", { "vertex_indices" }, 3); }
	catch (const std::exception& e) { std::cerr << "skipping: " << e.what() << std::endl; }

	// Tristrips must always be read with a 0 list size hint (unless you know exactly how many elements
	// are specifically in the file, which is unlikely); 
	try { _tripstrip = file.request_properties_from_element("tristrips", { "vertex_indices" }, 0); }
	catch (const std::exception& e) { std::cerr << "skipping " << e.what() << std::endl; }

	file.read(*file_stream);

	// now feed the data to the frame structure
	if (_vertices) {
		const size_t numVerticesBytes = _vertices->buffer.size_bytes();
		output.vertices.resize(_vertices->count * 3);
		std::memcpy(output.vertices.data(), _vertices->buffer.get(), numVerticesBytes);
	}
	if (_texcoords) {
		const size_t numTexcoordsBytes = _texcoords->buffer.size_bytes();
		output.uvcoords.resize(_texcoords->count * 2);
		std::memcpy(output.uvcoords.data(), _texcoords->buffer.get(), numTexcoordsBytes);
	}
	if (_normals) {
		const size_t numNormalsBytes = _normals->buffer.size_bytes();
		output.normals.resize(_normals->count * 3);
		std::memcpy(output.normals.data(), _normals->buffer.get(), numNormalsBytes);
	}
	if (_colors && _colors->t == tinyply::Type::UINT8) {
		const size_t numVerticesBytes = _colors->buffer.size_bytes();
		output.colors.resize(_colors->count * 3);
		for (size_t i = 0; i < _colors->buffer.size_bytes(); i++) {
			output.colors[i] = _colors->buffer.get()[i];
		}
	}
	if (_colorsRGBA && _colorsRGBA->t == tinyply::Type::UINT8) {
		const size_t numVerticesBytes = _colorsRGBA->buffer.size_bytes();
		output.colors.resize(_colorsRGBA->count * 4);
		for (size_t i = 0; i < _colorsRGBA->buffer.size_bytes(); i++) {
			output.colors[i] = _colorsRGBA->buffer.get()[i];
		}
	}

	if (_faces) {
		const size_t numVerticesBytes = _faces->buffer.size_bytes();
		output.triangles.resize(_faces->count * 3);
		std::memcpy(output.triangles.data(), _faces->buffer.get(), numVerticesBytes);
	}

	return true;

}

bool IO::_savePly(std::string filename, const Model& input)
{
	std::ofstream fout;
	// use a big 4MB buffer to accelerate writes
	char* buf = new char[4 * 1024 * 1024 + 1]; 
	fout.rdbuf()->pubsetbuf(buf, 4 * 1024 * 1024 + 1);
	fout.open(filename.c_str(), std::ios::out); 
	if (!fout)
	{
		std::cerr << "Error: can't open file " << filename << std::endl;
		delete[] buf;
		return false;
	}
	// this is mandatory to print floats with full precision
	fout.precision(std::numeric_limits< float >::max_digits10);

	fout << "ply" << std::endl;
	fout << "format ascii 1.0" << std::endl;
	fout << "comment Generated by InterDigital model processor" << std::endl;
	fout << "element vertex " << input.vertices.size() / 3 << std::endl;
	fout << "property float x" << std::endl;
	fout << "property float y" << std::endl;
	fout << "property float z" << std::endl;
	// normals
	if (input.normals.size() == input.vertices.size()) {
		fout << "property float nx" << std::endl;
		fout << "property float ny" << std::endl;
		fout << "property float nz" << std::endl;
	}
	// colors
	if (input.colors.size() == input.vertices.size()) {
		fout << "property uchar red" << std::endl;
		fout << "property uchar green" << std::endl;
		fout << "property uchar blue" << std::endl;
	}

	if (!input.triangles.empty()) {
		fout << "element face " << input.triangles.size() / 3 << std::endl;
		fout << "property list uchar int vertex_indices" << std::endl;
	}
	fout << "end_header" << std::endl;

	// comments
	for (int i = 0; i < input.comments.size(); i++) {
		fout << input.comments[i] << std::endl;
	}

	// vertices and colors
	for (int i = 0; i < input.vertices.size() / 3; i++) {
		fout <<
			input.vertices[i * 3 + 0] << " " <<
			input.vertices[i * 3 + 1] << " " <<
			input.vertices[i * 3 + 2] << " ";
		if (input.normals.size() == input.vertices.size()) {
			fout <<
				input.normals[i * 3 + 0] << " " <<
				input.normals[i * 3 + 1] << " " <<
				input.normals[i * 3 + 2] << " ";
		}
		if (input.colors.size() == input.vertices.size()) {
			// do not cast as char otherwise characters are printed instead if int8 values
			fout <<
				(unsigned short)(std::roundf(input.colors[i * 3 + 0])) << " " <<
				(unsigned short)(std::roundf(input.colors[i * 3 + 1])) << " " <<
				(unsigned short)(std::roundf(input.colors[i * 3 + 2]));
		}
		fout << std::endl;
	}
	// topology
	for (int i = 0; i < input.triangles.size() / 3; i++) {
		fout << "3 "
			<< input.triangles[i * 3 + 0] << " "
			<< input.triangles[i * 3 + 1] << " "
			<< input.triangles[i * 3 + 2] << std::endl;
	}

	fout.close();
	delete[] buf;
	return true;
}

bool IO::_loadImage(std::string filename, Image& output) {

	// Reading map if needed
	if (filename != "") {
		std::cout << "Input map: " << filename << std::endl;
		output.data = stbi_load(filename.c_str(), &output.width, &output.height, &output.nbc, 0);
		if ( output.data == NULL ){
			std::cout << "Error: opening file " << filename << std::endl;
			return false;
		}
	}
	else {
		std::cout << "Error: invalid empty filename" << std::endl;
		return false;
	}

	// success
	return true;
}

bool IO::_loadImageFromVideo(std::string filename, Image& output) {

	// Reading map if needed
	if (filename != "") {
		// parsing filename to extract metadata
		// int frIdx, int width, int height, bool isYUV, bool is444,
		size_t      pos = filename.find_last_of(".");
		std::string extension = filename.substr(pos);
		bool isYUV = (extension.compare(".yuv") == 0);
		size_t      pos2 = filename.find_last_of("_");
		std::string codingType = filename.substr(pos2 + 1, pos - pos2 - 1);
		bool is444 = (codingType.compare("yuv420p") != 0);
		size_t      pos3 = filename.substr(0, pos2 - 1).find_last_of("_");
		std::string frameDimension = filename.substr(0, pos2).substr(pos3 + 1);
		int width = std::stoi(frameDimension.substr(0, frameDimension.find('x')));
		int height = std::stoi(frameDimension.substr(frameDimension.find('x') + 1));
		int frameIndex = _context->getFrame() - _context->getFirstFrame();
		// 
		std::cout << "Reading video frame "<< frameIndex <<" from file: " << filename << std::endl;
		int chromaStride = is444? width : width/2;
		int chromaHeight = is444? height : height/2;
		int frSize = (height*width+2*chromaStride*chromaHeight);
		//open the video file and search for the frame index
        std::ifstream in;
        in.open(filename, std::ifstream::in | std::ios::binary);
        if (!in.is_open()) { return false; }
        in.seekg(frameIndex * frSize, std::ios::beg);
		//reading frame (only 8-bit data is allowed at this moment)
        char* frame = new char[frSize];
        in.read(frame, frSize);
        in.close();
		//now convert the frame
		if(!is444){
			//chroma upsampling using nearest neighbor
			char* frameUpscaled = new char[3*width*height];
			//copy the luma channel
			memcpy(frameUpscaled,frame,width*height);
			//copy the down-sampled chroma channel
			for(int y=0;y<chromaHeight;y++)
				for(int x=0;x<chromaStride;x++){
					frameUpscaled[width*height + ((2*x) + width*(2*y))] = frame[width*height + (x + chromaStride*y)];
					frameUpscaled[width*height + ((2*x+1) + width*(2*y))] = frame[width*height + (x + chromaStride*y)];
					frameUpscaled[width*height + ((2*x) + width*(2*y+1))] = frame[width*height + (x + chromaStride*y)];
					frameUpscaled[width*height + ((2*x+1) + width*(2*y+1))] = frame[width*height + (x + chromaStride*y)];
				}

			//copy the down-sampled chroma channel
			for(int y=0;y<chromaHeight;y++)
				for(int x=0;x<chromaStride;x++){
					frameUpscaled[2*width*height + ((2*x) + width*(2*y))] = frame[width*height + chromaStride*chromaHeight + (x + chromaStride*y)];
					frameUpscaled[2*width*height + ((2*x+1) + width*(2*y))] = frame[width*height + chromaStride*chromaHeight + (x + chromaStride*y)];
					frameUpscaled[2*width*height + ((2*x) + width*(2*y+1))] = frame[width*height + chromaStride*chromaHeight + (x + chromaStride*y)];
					frameUpscaled[2*width*height + ((2*x+1) + width*(2*y+1))] = frame[width*height + chromaStride*chromaHeight + (x + chromaStride*y)];
				}
			delete[] frame;
			frame = frameUpscaled;
		}
		output.data = new unsigned char[3*width*height];
		output.height = height;
		output.width = width;
		output.nbc = 3;
		if(isYUV){
			//convert to RGB
			for(int y=0;y<height;y++){
				for(int x=0;x<width;x++){
					double Y = (unsigned char)frame[x + width*y];
					Y = std::min<double>(std::max<double>(Y/255.0,0.0),1.0);
					double Cb = (unsigned char)frame[width*height + x + width*y];
					Cb = std::min<double>(std::max<double>((Cb-128)/255.0,-0.5),0.5);
					double Cr = (unsigned char)frame[2*width*height + x + width*y];
					Cr = std::min<double>(std::max<double>((Cr-128)/255.0,-0.5),0.5);
					double R = Y + 1.57480 * Cr;
					output.data[(x + width*y)*3 + 0] = (unsigned char)std::round(255*std::min<double>(std::max<double>(R, 0.0),1.0));
					double G = Y - 0.18733 * Cb - 0.46813 * Cr;
					output.data[(x + width*y)*3 + 1] = (unsigned char)std::round(255*std::min<double>(std::max<double>(G, 0.0),1.0));
					double B = Y + 1.85563 * Cb;
					output.data[(x + width*y)*3 + 2] = (unsigned char)std::round(255*std::min<double>(std::max<double>(B, 0.0),1.0));
				}
			}
		}
		else{
			//is GBR, so re-order the color planes
			for(int y=0;y<height;y++){
				for(int x=0;x<width;x++){
					output.data[(x + width*y)*3 + 0] = frame[(x + width*y) + 2*width*height];
					output.data[(x + width*y)*3 + 2] = frame[(x + width*y) + width*height]; 
					output.data[(x + width*y)*3 + 1] = frame[(x + width*y)];
				}
			}
		}
		delete[] frame;
	}
	else {
		std::cout << "Error: invalid empty filename" << std::endl;
		return false;
	}

	// success
	return true;
}