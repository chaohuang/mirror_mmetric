﻿// ************* COPYRIGHT AND CONFIDENTIALITY INFORMATION *********
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
// Author: meshToPcFace method based on original face sampling code from Owlii
// *****************************************************************

#include <iostream>
#include <fstream>
#include <unordered_map>
#include <time.h>
#include <math.h>
// mathematics
#include <glm/vec3.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
// argument parsing
#include <cxxopts.hpp>

// internal headers
#include "mmGeometry.h"
#include "mmIO.h"
#include "mmSample.h"
#include "mmModel.h"
#include "mmImage.h"

const char* Sample::brief = "Convert mesh to point cloud";

// analyse command line and run processings
int Sample::main(std::string app, std::string cmd, int argc, char* argv[])
{
	// the command options
	std::string inputModelFilename;
	std::string inputTextureFilename;
	std::string outputModelFilename;
	bool hideProgress = false;
	// the type of processing
	// in "face", "grid", "map"
	std::string mode = "face";
	// Face options
	size_t resolution = 1024;
	float thickness = 0.0;
	// Grid options
	int gridSize = 1024;
	// Face and Grid options
	bool bilinear = false;

	// command line parameters
	try
	{
		cxxopts::Options options(app + " sample", Sample::brief);
		options.add_options()
			("i,inputModel", "path to input model (obj or ply file)",
				cxxopts::value<std::string>())
			("m,inputMap", "path to input texture map (png, jpeg)",
				cxxopts::value<std::string>())
			("o,outputModel", "path to output model (obj or ply file)",
				cxxopts::value<std::string>())
			("mode", "the sampling mode in [face,grid,map]",
				cxxopts::value<std::string>())
			("hideProgress", "hide progress display in console for use by robot",
				cxxopts::value<bool>()->default_value("false"))
			("h,help", "Print usage")
			;
		options.add_options("Face mode")
			("float", "if set the processings and outputs will be float32, int32 otherwise",
				cxxopts::value<bool>()->default_value("true"))
			("resolution", "integer value in [1,maxuint], nb samples per edge of maximal size",
				cxxopts::value<size_t>()->default_value("1024"))
			("thickness", "floating point value, distance to border of the face",
				cxxopts::value<float>()->default_value("0.0"))
			;
		options.add_options("Grid mode")
			("gridSize", "integer value in [1,maxint], side size of the grid",
				cxxopts::value<int>()->default_value("1024"))
			;
		options.add_options("Grid and Face modes")
			("bilinear", "if set, texture filtering will be bilinear, nearest otherwise",
				cxxopts::value<bool>()->default_value("true"))
			;

		auto result = options.parse(argc, argv);

		// Analyse the options
		if (result.count("help") || result.arguments().size() == 0)
		{
			std::cout << options.help() << std::endl;
			return 0;
		}
		//	
		if (result.count("inputModel"))
			inputModelFilename = result["inputModel"].as<std::string>();
		else {
			std::cerr << "Error: missing inputModel parameter" << std::endl;
			std::cout << options.help() << std::endl;
			return 1;
		}
		//
		if (result.count("inputMap"))
			inputTextureFilename = result["inputMap"].as<std::string>();
		//
		if (result.count("outputModel"))
			outputModelFilename = result["outputModel"].as<std::string>();
		else {
			std::cerr << "Error: missing outputModel parameter" << std::endl;
			std::cout << options.help() << std::endl;
			return 1;
		}
		//
		if (result.count("mode"))
			mode = result["mode"].as<std::string>();
		//
		if (result.count("hideProgress"))
			hideProgress = result["hideProgress"].as<bool>();
		//
		if (result.count("resolution"))
			resolution = result["resolution"].as<size_t>();
		if (result.count("thickness"))
			thickness = result["thickness"].as<float>();
		if (result.count("gridSize"))
			gridSize = result["gridSize"].as<int>();
		if (result.count("bilinear"))
			bilinear = result["bilinear"].as<bool>();
	}
	catch (const cxxopts::OptionException& e)
	{
		std::cout << "error parsing options: " << e.what() << std::endl;
		return 1;
	}

	// Reading map if needed
	Image textureMap;
	if (inputTextureFilename != "") {
		IO::loadImage(inputTextureFilename, textureMap);
	}
	else {
		std::cout << "Skipping map read, will parse use vertex color if any" << std::endl;
	}

	// the input
	Model inputModel;
	if (!IO::loadModel(inputModelFilename, inputModel)) {
		return 1;
	} 
	if (inputModel.vertices.size() == 0 ||
		inputModel.triangles.size() == 0) {
		std::cout << "Error: invalid input model from " << inputModelFilename << std::endl;
		return 1;
	}

	// the output
	Model outputModel;

	// Perform the processings
	clock_t t1 = clock();
	if (mode == "face") {
		std::cout << "Sampling in FACE mode" << std::endl;
		std::cout << "  Resolution = " << resolution << std::endl;
		std::cout << "  Thickness = " << thickness << std::endl;
		std::cout << "  Bilinear = " << bilinear << std::endl;
		std::cout << "  hideProgress = " << hideProgress << std::endl;
		Sample::meshToPcFace(inputModel, outputModel,
			textureMap, resolution, thickness, bilinear, !hideProgress);
	}
	else if (mode == "grid") {
		std::cout << "Sampling in GRID mode" << std::endl;
		std::cout << "  Grid Size = " << gridSize << std::endl;
		std::cout << "  Bilinear = " << bilinear << std::endl;
		std::cout << "  hideProgress = " << hideProgress << std::endl;
		Sample::meshToPcGrid(inputModel, outputModel,
			textureMap, gridSize, bilinear, !hideProgress);
	}
	else if (mode == "map") {
		std::cout << "Sampling in MAP mode" << std::endl;
		std::cout << "  hideProgress = " << hideProgress << std::endl;
		Sample::meshToPcMap(inputModel, outputModel, textureMap, !hideProgress);
	}
	clock_t t2 = clock();
	std::cout << "Time on processing: " << ((float)(t2 - t1)) / CLOCKS_PER_SEC << " sec." << std::endl;

	// save the result
	if (IO::saveModel(outputModelFilename, outputModel))
		return 0;
	else
		return 1;

}

// this algorithm was originally developped by Owlii
void Sample::meshToPcFace(
	const Model& input, Model& output,
	const Image& tex_map, size_t resolution, float thickness, bool bilinear, bool logProgress) {

	// computes the bounding box of the vertices
	glm::vec3 minPos, maxPos;
	computeBBox(input.vertices, minPos, maxPos);
	std::cout << "minbox = " << minPos[0] << "," << minPos[1] << "," << minPos[2] << std::endl;
	std::cout << "maxbox = " << maxPos[0] << "," << maxPos[1] << "," << maxPos[2] << std::endl;

	// computes the sampling step
	glm::vec3 diag = maxPos - minPos;
	float boxMaxSize = std::max(diag.x, std::max(diag.y, diag.z));
	float step = boxMaxSize / resolution;
	std::cout << "step = " << step << std::endl;

	output.vertices.clear();
	output.colors.clear();
	size_t skipped = 0; // number of degenerate triangles

	for (size_t t = 0; t < input.triangles.size() / 3; t++) {

		if (logProgress)
			std::cout << '\r' << t << "/" << input.triangles.size() / 3 << std::flush;

		// fetch the triangle vertices
		glm::vec3 v1, v2, v3;
		input.fetchTriangleVertices(t, v1, v2, v3);
		// fetch the triangle UVs if needed
		glm::vec2 uv1, uv2, uv3;
		if (input.uvcoords.size() != 0) {
			input.fetchTriangleUVs(t, uv1, uv2, uv3);
		}
		// fetch the triangle colors if needed
		glm::vec3 c1 = { 0.0F,0.0F,0.0F };
		glm::vec3 c2 = c1; glm::vec3 c3 = c1;
		if (input.colors.size() != 0) {
			input.fetchTriangleColors(t, c1, c2, c3);
		}
		// check if triangle is not degenerate
		if (triangleArea(v1, v2, v3) < DBL_EPSILON) {
			++skipped;
			continue;
		}
		// computes face dimensions for sampling
		glm::vec3 v12_norm = v2 - v1;
		glm::vec3 v23_norm = v3 - v2;
		float l12 = glm::length(v12_norm);
		float l23 = glm::length(v23_norm);
		for (int i = 0; i < 3; i++) {
			v12_norm[i] = v12_norm[i] / l12;
			v23_norm[i] = v23_norm[i] / l23;
		}
		// compute face normal
		glm::vec3 normal;
		triangleNormal(v1, v2, v3, normal);

		// do the sampling
		for (float step12 = 0.f; step12 <= l12; step12 += step) {
			for (float step23 = 0.f; step23 <= step12 / l12 * l23; step23 += step) {

				float step_normal_bdry = 0.f;
				while (step_normal_bdry <= thickness) {
					step_normal_bdry += step;
				}
				step_normal_bdry -= step;

				for (float step_normal = -step_normal_bdry; step_normal <= step_normal_bdry; step_normal += step) {

					float point[3] = { 0.f, 0.f, 0.f };
					for (int i = 0; i < 3; i++) {
						point[i] = v1[i] + step12 * v12_norm[i] + step23 * v23_norm[i] + step_normal * normal[i];
					}

					// push the position and normal
					for (glm::vec3::length_type c = 0; c < 3; c++) {
						output.vertices.push_back(point[c]);
						output.normals.push_back(normal[c]);
					}

					// compute the color
					glm::vec3 rgb = { 0.0F,0.0F,0.0F };
					bool push = false;

					if (input.uvcoords.size() != 0 && tex_map.data != NULL) { // use the texture map
						push = true;
						// compute UV
						const glm::vec2 uv{
							(uv1[0] + step12 / l12 * (uv2[0] - uv1[0]) + step23 / l23 * (uv3[0] - uv2[0])),
							(uv1[1] + step12 / l12 * (uv2[1] - uv1[1]) + step23 / l23 * (uv3[1] - uv2[1]))
						};

						// fetch the color from the map
						if (bilinear)
							texture2D_bilinear(tex_map, uv, rgb);
						else
							texture2D(tex_map, uv, rgb);
					}
					else if (input.colors.size() != 0) { // use color per vertex
						push = true;
						rgb[0] = c1[0] + step12 / l12 * (c2[0] - c1[0]) + step23 / l23 * (c3[0] - c2[0]);
						rgb[1] = c1[1] + step12 / l12 * (c2[1] - c1[1]) + step23 / l23 * (c3[1] - c2[1]);
						rgb[2] = c1[2] + step12 / l12 * (c2[2] - c1[2]) + step23 / l23 * (c3[2] - c2[2]);
					}

					// add color to the table of points if needed
					if (push) {
						for (glm::vec3::length_type i = 0; i < 3; i++)
							output.colors.push_back(rgb[i]);
					}
				}
			}
		}
	}
	if (logProgress)
		std::cout << std::endl;
	if (skipped != 0)
		std::cout << "Skipped " << skipped << " triangles" << std::endl;
	std::cout << "Generated " << output.vertices.size() / 3 << " points" << std::endl;
}

// we use ray tracing to process the result, we could also use a rasterization (might be faster)
void Sample::meshToPcGrid(
	const Model& input, Model& output,
	const Image& tex_map, size_t resolution, bool bilinear, bool logProgress) {

	// computes the bounding box of the vertices
	glm::vec3 minPos, maxPos;
	computeBBox(input.vertices, minPos, maxPos);
	std::cout << "minbox = " << minPos[0] << "," << minPos[1] << "," << minPos[2] << std::endl;
	std::cout << "maxbox = " << maxPos[0] << "," << maxPos[1] << "," << maxPos[2] << std::endl;
	
	std::cout << "Transform bounding box to square box" << std::endl;
	// hence sampling will be unform in the three dimensions
	toCubicalBBox(minPos, maxPos);
	std::cout << "minbox = " << minPos[0] << "," << minPos[1] << "," << minPos[2] << std::endl;
	std::cout << "maxbox = " << maxPos[0] << "," << maxPos[1] << "," << maxPos[2] << std::endl;

	// we will now sample between min and max over the three dimensions, using resolution
	// by throwing rays from the three orthogonal faces of the box XY, XZ, YZ

	size_t skipped = 0; // number of degenerate triangles

	// for each triangle
	for (size_t triIdx = 0; triIdx < input.triangles.size() / 3; ++triIdx) {

		if (logProgress)
			std::cout << '\r' << triIdx << "/" << input.triangles.size() / 3 << std::flush;

		// fetch the triangle vertices
		glm::vec3 v1, v2, v3;
		input.fetchTriangleVertices(triIdx, v1, v2, v3);
		// fetch the triangle UVs if needed
		glm::vec2 uv1, uv2, uv3;
		if (input.uvcoords.size() != 0) {
			input.fetchTriangleUVs(triIdx, uv1, uv2, uv3);
		}
		// fetch the triangle colors if needed
		glm::vec3 c1 = { 0.0F,0.0F,0.0F };
		glm::vec3 c2 = c1; glm::vec3 c3 = c1;
		if (input.colors.size() != 0) {
			input.fetchTriangleColors(triIdx, c1, c2, c3);
		}
		// check if triangle is not degenerate
		if (triangleArea(v1, v2, v3) < DBL_EPSILON) {
			++skipped;
			continue;
		}
		// compute face normal
		glm::vec3 normal;
		triangleNormal(v1, v2, v3, normal);
		// extract the triangle bbox
		glm::vec3 triMinPos, triMaxPos;
		triangleBBox(v1, v2, v3, triMinPos, triMaxPos);

		// now find the Discrete range from global box to triangle box
		glm::vec3 stepSize = (maxPos - minPos) * (1.0F / (float)(resolution - 1));
		glm::vec3 lmin = glm::floor((triMinPos - minPos) / stepSize); // can lead to division by zero with flat box, handled later
		glm::vec3 lmax = glm::ceil((triMaxPos - minPos) / stepSize); // idem
		glm::vec3 lcnt = lmax - lmin;

		// now we will send rays on this triangle from the discreet steps of this box
		// rayTrace from the three main axis
		for (glm::vec3::length_type mainAxis = 0; mainAxis < 3; ++mainAxis) {

			// axis swizzling
			glm::vec3::length_type secondAxis = 1;	glm::vec3::length_type thirdAxis = 2;
			if (mainAxis == 1) { secondAxis = 0; thirdAxis = 2; }
			else if (mainAxis == 2) { secondAxis = 0; thirdAxis = 1; }

			// skip this axis if box is null sized on one of the two other axis
			if (minPos[secondAxis] == maxPos[secondAxis] || minPos[thirdAxis] == maxPos[thirdAxis])
				continue;

			// let's thow from mainAxis prependicular plane
			glm::vec3 rayOrigin = { 0.0,0.0,0.0 };
			glm::vec3 rayDirection = { 0.0,0.0,0.0 };

			// on the main axis
			if (stepSize[mainAxis] == 0.0F) { // handle stepSize[axis]==0
				// add small thress to be sure ray intersect in positive t
				rayOrigin[mainAxis] = minPos[mainAxis] - 0.5F;
			}
			else {
				rayOrigin[mainAxis] = minPos[mainAxis] + lmin[mainAxis] * stepSize[mainAxis];
			}
			// on main axis from min to max
			rayDirection[mainAxis] = 1.0;

			// iterate the second axis with i 
			for (size_t i = 0; i <= lcnt[secondAxis]; ++i) {

				// iterate the third axis with j
				for (size_t j = 0; j <= lcnt[thirdAxis]; ++j) {

					// create the ray, starting from the face of the triangle bbox
					rayOrigin[secondAxis] = minPos[secondAxis] + (lmin[secondAxis] + i) * stepSize[secondAxis];
					rayOrigin[thirdAxis] = minPos[thirdAxis] + (lmin[thirdAxis] + j) * stepSize[thirdAxis];

					//  triplet, x = t, y = u, z = v with t the parametric and (u,v) the barycentrics
					glm::vec3 res;

					// let' throw the ray toward the triangle
					if (evalRayTriangle(rayOrigin, rayDirection, v1, v2, v3, res)) {

						// we convert the result into a point with color

						// push the position and normal
						glm::vec3 point = rayOrigin + rayDirection * res[0];
						for (glm::vec3::length_type c = 0; c < 3; c++) {
							output.vertices.push_back(point[c]);
							output.normals.push_back(normal[c]);
						}

						// compute the color
						glm::vec3 rgb = { 0.0F,0.0F,0.0F };
						bool push = false;

						// use the texture map
						if (input.uvcoords.size() != 0 && tex_map.data != NULL) {
							push = true;
							// use barycentric coordinates to extract point UV
							glm::vec2 uv = uv1 * (1.0f - res.y - res.z) + uv2 * res.y + uv3 * res.z;

							// fetch the color from the map
							if (bilinear)
								texture2D_bilinear(tex_map, uv, rgb);
							else
								texture2D(tex_map, uv, rgb);
						}
						// use color per vertex
						else if (input.colors.size() != 0) {
							push = true;
							// compute pixel color using barycentric coordinates
							rgb = c1 * (1.0f - res.y - res.z) + c2 * res.y + c3 * res.z;
						}

						// add color to the table of points
						if (push) {
							for (int i = 0; i < 3; i++)
								output.colors.push_back(rgb[i]);
						}
					}
				}
			}
		}
	}
	if (logProgress)
		std::cout << std::endl;
	if (skipped != 0)
		std::cout << "Skipped " << skipped << " triangles" << std::endl;
	std::cout << "Generated " << output.vertices.size() / 3 << " points" << std::endl;

}

// perform a reverse sampling of the texture map to generate mesh samples
// the color of the point is then using the texel color => no filtering
void Sample::meshToPcMap(
	const Model& input, Model& output,
	const Image& tex_map, bool logProgress) {

	if (input.trianglesuv.size() == 0 || input.uvcoords.size() == 0) {
		std::cerr << "Error: cannot back sample model, no UV coordinates" << std::endl;
		return;
	}
	if (tex_map.width <= 0 || tex_map.height <= 0 || tex_map.nbc < 3 || tex_map.data == NULL) {
		std::cerr << "Error: cannot back sample model, no valid texture map" << std::endl;
		return;
	}

	size_t skipped = 0; // number of degenerate triangles

	// For each triangle
	for (size_t triIdx = 0; triIdx < input.triangles.size() / 3; ++triIdx) {

		if (logProgress)
			std::cout << '\r' << triIdx << "/" << input.triangles.size() / 3 << std::flush;

		// fetch the triangle vertices
		glm::vec3 v1, v2, v3;
		input.fetchTriangleVertices(triIdx, v1, v2, v3);
		// compute face normal
		glm::vec3 normal;
		triangleNormal(v1, v2, v3, normal);
		// fetch the triangle UVs if needed
		glm::vec2 uv1, uv2, uv3;
		input.fetchTriangleUVs(triIdx, uv1, uv2, uv3);
		// check if triangle is not degenerate
		if (triangleArea(v1, v2, v3) < DBL_EPSILON) {
			++skipped;
			continue;
		}

		// compute the UVs bounding box
		glm::vec2 uvMin = { FLT_MAX, FLT_MAX };
		glm::vec2 uvMax = { -FLT_MAX, -FLT_MAX };
		uvMin = glm::min(uv1, uvMin);
		uvMin = glm::min(uv2, uvMin);
		uvMin = glm::min(uv3, uvMin);
		uvMax = glm::max(uv1, uvMax);
		uvMax = glm::max(uv2, uvMax);
		uvMax = glm::max(uv3, uvMax);

		// find the integer coordinates covered in the map
		glm::i32vec2 intUvMin = { (tex_map.width - 1) * uvMin.x, (tex_map.height - 1) * uvMin.y };
		glm::i32vec2 intUvMax = { (tex_map.width - 1) * uvMax.x, (tex_map.height - 1) * uvMax.y };
		;
		// loop over the box in image space
		// if a pixel center is in the triangle then backproject
		// and create a new point with the pixel color
		for (size_t i = intUvMin[0]; i <= intUvMax[0]; ++i) {
			for (size_t j = intUvMin[1]; j <= intUvMax[1]; ++j) {

				// get the UV for the center of the pixel
				glm::vec2 pixelUV = { (0.5F + i) / tex_map.width, (0.5F + j) / tex_map.height };
				// test if this pixelUV is in the triangle UVs
				glm::vec3 bary; // the barycentrics if success
				if (getBarycentric(pixelUV, uv1, uv2, uv3, bary)) {
					// revert pixelUV to find point in 3D
					glm::vec3 point;
					triangleInterpolation(v1, v2, v3, bary.x, bary.y, point);
					// push the position and normal
					for (glm::vec3::length_type c = 0; c < 3; c++) {
						output.vertices.push_back(point[c]);
						output.normals.push_back(normal[c]);
					}
					// fetch the color
					glm::vec3 rgb = { 0.0F,0.0F,0.0F };
					if (true) { // set false for debug, color chanel will contain the UVs
						texture2D(tex_map, pixelUV, rgb);
					}
					else {
						rgb[0] = std::round(pixelUV.x * 255);
						rgb[2] = std::round(pixelUV.y * 255);
					}
					// add color to the table of points
					for (int i = 0; i < 3; i++)
						output.colors.push_back(rgb[i]);
				}
			}
		}
	}
	if (logProgress)
		std::cout << std::endl;
	if (skipped != 0)
		std::cout << "Skipped " << skipped << " triangles" << std::endl;
	std::cout << "Generated " << output.vertices.size() / 3 << " points" << std::endl;
}