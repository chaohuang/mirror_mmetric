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
// Author: loadObj/saveObj methods based on original code from Owlii
// *****************************************************************

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

// create the stores
std::map<std::string, Model*> IO::_models;
std::map<std::string, Image*> IO::_images;

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
		std::cout << "Input model: " << filename << std::endl;
		std::cout << "  Vertices: " << output.vertices.size() / 3 << std::endl;
		std::cout << "  UVs: "      << output.uvcoords.size() / 2 << std::endl;
		std::cout << "  Colors: "     << output.colors.size() / 3 << std::endl;
		std::cout << "  Normals: " << output.normals.size() / 3 << std::endl;
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
	fin.open(filename.c_str(), std::ios::in);
	if (!fin)
	{
		std::cerr << "Error: can't open file " << filename << std::endl;
		return false;
	}
	int temp_index;
	float temp_pos, temp_uv, temp_normal, temp_col;
	std::string temp_flag, temp_str;
	std::string line;
	std::getline(fin, line);
	while (fin) {
		std::istringstream in(line);
		in >> temp_flag;  // temp_flag: the first word in the line
		if (temp_flag.compare(std::string("mtllib")) == 0) {
			output.header = std::string(line.c_str());
		}
		if (temp_flag.compare(std::string("v")) == 0) {
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

	return true;
}

bool IO::_saveObj(std::string filename, const Model& input) {

	std::ofstream fout;
	fout.open(filename.c_str(), std::ios::out); // TODO check error
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
		fout << "f " <<
			input.triangles[i * 3 + 0] + 1 << "/" << input.triangles[i * 3 + 0] + 1 << " " <<
			input.triangles[i * 3 + 1] + 1 << "/" << input.triangles[i * 3 + 1] + 1 << " " <<
			input.triangles[i * 3 + 2] + 1 << "/" << input.triangles[i * 3 + 2] + 1 << std::endl;
	}
	fout.close();

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
	fout.open(filename.c_str(), std::ios::out); // tod check for error
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
	//yo- if ((input.colors.size() / 4) == (input.vertices.size() / 3) )
	//yo-   fout << "property uchar alpha" << std::endl;
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

	return true;
}

bool IO::_loadImage(std::string filename, Image& output) {

	// Reading map if needed
	if (filename != "") {
		std::cout << "Input map: " << filename << std::endl;
		output.data = stbi_load(filename.c_str(), &output.width, &output.height, &output.nbc, 3);
		if (output.nbc != 3) {
			std::cout << "Error: texture map shall be 3 channels per component, got " << output.nbc << std::endl;
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