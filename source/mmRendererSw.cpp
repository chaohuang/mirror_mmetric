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

#include <map>
#include <iostream>
#include <time.h>

// "implementation" done in mmRendererHW
#include <stb_image_write.h>

// mathematics
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtc/matrix_inverse.hpp>

// internal headers
#include "mmIO.h"
#include "mmModel.h"
#include "mmImage.h"
#include "mmGeometry.h"
#include "mmRendererSw.h"

glm::mat4 modelView;	 // Model View matrix
glm::mat4 mvp;			 // Model View Projection
glm::mat4 normalMatrix;  // invert_transpose( modelView )
glm::mat4 vp;			 // viewport
glm::vec3 viewPosition;  // viewpoint
glm::vec3 viewPositionMV; 
glm::vec3 viewPositionMVP;
glm::vec3 lightPosition;
glm::vec3 lightPositionMV;
glm::vec3 lightColor;
glm::vec3 materialAmbient;
glm::vec3 materialDiffuse;

bool isCullingEnabled;
bool cwCulling;

struct IShader {

	const Model* model;
	const Image* map;

	IShader(const Model* model, const Image* map) :
		model(model), map(map) {}

	// vertex program
	virtual glm::vec4 vertex(const int iface, const int nthvert) = 0;
	// fragment program
	virtual bool fragment(const glm::vec3 bar, glm::vec4& color) = 0;
};

struct ShaderMap :IShader {

	glm::mat3x2 varying_uv;  // triangle uv coordinates, written by the vertex shader, read by the fragment shader

	ShaderMap(const Model* model, const Image* map) : IShader(model, map) {}

	glm::vec4 vertex(const int iface, const int nthvert) {
		// fetch uv coordinates
		varying_uv = glm::column(varying_uv, nthvert, model->fetchUv(iface, nthvert));
		// fetch and transform the vertex position 
		glm::vec4 gl_Vertex = mvp * glm::vec4(model->fetchPosition(iface, nthvert), 1.0F);
		return gl_Vertex;
	}

	bool fragment(const glm::vec3 bar, glm::vec4& color) {
		// tex coord interpolation
		glm::vec2 uv = varying_uv * bar;
		// texel fetch
		glm::vec3 rgb;
		texture2D_bilinear(*map, uv, rgb); // we know map != NULL
		color = glm::vec4(rgb.r, rgb.g, rgb.b, 255);
		// the pixel is not discard
		return false;
	}
};

struct ShaderMapLight :IShader {

	glm::mat3x2 varying_uv;   // triangle uv coordinates, written by the vertex shader, read by the fragment shader
	glm::mat3x3 varying_nrm;  // normal per vertex to be interpolated by FS
	glm::mat3x3 varying_vert; // vertex interpolation in model view

	ShaderMapLight(const Model* model, const Image* map) : IShader(model, map) {}

	glm::vec4 vertex(const int iface, const int nthvert) {
		// fetch uv coordinates
		varying_uv = glm::column(varying_uv, nthvert, model->fetchUv(iface, nthvert));
		varying_nrm = glm::column(varying_nrm, nthvert, glm::vec3(normalMatrix * glm::vec4(model->fetchNormal(iface, nthvert), 1.0)));
		// fetch and transform the vertex position 
		glm::vec4 pos(model->fetchPosition(iface, nthvert), 1.0);
		varying_vert = glm::column(varying_vert, nthvert, glm::vec3(modelView * pos));
		glm::vec4 gl_Vertex = mvp * pos;
		return gl_Vertex;
	}

	bool fragment(const glm::vec3 bar, glm::vec4& color) {
		// tex coord interpolation
		glm::vec2 uv = varying_uv * bar;
		// normal interpolation
		glm::vec3 nrm = glm::normalize(varying_nrm * bar);
		// vertex coord interpolation
		glm::vec3 vert = varying_vert * bar;
		// texel fetch
		glm::vec3 rgb;
		texture2D_bilinear(*map, uv, rgb); // we know map != NULL, rgb is in 0-255 for each component
		// compute the lighting
		// ambient term
		glm::vec3 Iamb = rgb * materialAmbient;
		// diffuse term Kd = 0.5 * map(uv)
		glm::vec3 L = glm::normalize(lightPositionMV - vert);
		glm::vec3 Idiff = (rgb * materialDiffuse) * (lightColor * std::max(glm::dot(nrm, L), 0.0F));
		// store the result
		color = glm::vec4(glm::clamp(Iamb + Idiff, 0.0F, 255.0F), 255);
		// uncomment to visualize normals
		// color = glm::vec4(nrm * 255.0F, 255.0F);
		// the pixel is not discard
		return false;
	}
};

struct ShaderCpv :IShader {

	glm::mat3x3 varying_color;

	ShaderCpv(const Model* model, const Image* map) : IShader(model, map) {}

	glm::vec4 vertex(const int iface, const int nthvert) {
		// fetch per vertex colors
		varying_color = glm::column(varying_color, nthvert, model->fetchColor(iface, nthvert));
		// fetch and transform the vertex position 
		glm::vec4 gl_Vertex = mvp * glm::vec4(model->fetchPosition(iface, nthvert), 1.0F);
		return gl_Vertex;
	}

	bool fragment(const glm::vec3 bar, glm::vec4& color) {
		// tex coord interpolation
		color = glm::vec4(varying_color * bar, 255);
		// the pixel is not discard
		return false;
	}
};

struct ShaderRed :IShader {

	ShaderRed(const Model* model, const Image* map) : IShader(model, map) {}

	glm::vec4 vertex(const int iface, const int nthvert) {
		// fetch and transform the vertex position 
		glm::vec4 gl_Vertex = mvp * glm::vec4(model->fetchPosition(iface, nthvert), 1.0F);
		return gl_Vertex;
	}

	bool fragment(const glm::vec3 bar, glm::vec4& color) {
		// tex coord interpolation
		color = glm::vec4(255, 0, 0, 255);
		// the pixel is not discard
		return false;
	}
};

void viewport(const int x, const int y, const int w, const int h) {
	vp = glm::mat4(
		glm::vec4(w / 2., 0, x + w / 2., 0),
		glm::vec4(0, h / 2., y + h / 2., 0),
		glm::vec4(0, 0, 1, 0),
		glm::vec4(0, 0, 0, 1));
}

void rasterize(IShader& shader, std::vector<uint8_t>& fbuffer, int width, int height, std::vector<float>& zbuffer) {

	// for every triangle
	for (int triIdx = 0; triIdx < shader.model->triangles.size() / 3; ++triIdx) {
				
		// triangle coordinates (clip coordinates), written by VS, read by FS
		glm::vec4 clip_verts[3];

		// call the vertex shader for each triangle vertex
		for (int vertIdx = 0; vertIdx < 3; ++vertIdx) {
			clip_verts[vertIdx] = shader.vertex(triIdx, vertIdx);
		}
		
		// backface culling 
		// only works for ortho projection
		// might use pts2 (i.e. coords in box) instead to support perspective - to be checked
		if (isCullingEnabled) {
			glm::vec3 normal = glm::cross(glm::vec3(clip_verts[1] - clip_verts[0]), glm::vec3(clip_verts[2] - clip_verts[0]));
			if ((cwCulling && normal.z <= 0) || (!cwCulling && normal.z >= 0)) continue;
		}

		// triangle screen coordinates before persp. division
		glm::vec4 pts[3] = { vp * clip_verts[0],    vp * clip_verts[1],    vp * clip_verts[2] };

		// triangle screen coordinates after  persp. division
		glm::vec2 pts2[3] = { glm::vec2(pts[0] / pts[0][3]), glm::vec2(pts[1] / pts[1][3]), glm::vec2(pts[2] / pts[2][3]) };

		// compute the rasterization box
		glm::vec2 bboxmin(std::numeric_limits<double>::max(), std::numeric_limits<double>::max());
		glm::vec2 bboxmax(-std::numeric_limits<double>::max(), -std::numeric_limits<double>::max());
		glm::vec2 clamp(width - 1, height - 1);

		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 2; j++) {
				bboxmin[j] = std::max(0.0F, std::min(bboxmin[j], pts2[i][j]));
				bboxmax[j] = std::min(clamp[j], std::max(bboxmax[j], pts2[i][j]));
			}
		}

		// rasterize
		for (int x = (int)bboxmin.x; x <= (int)bboxmax.x; x++) {
			for (int y = (int)bboxmin.y; y <= (int)bboxmax.y; y++) {

				// compute barycentric coordinates in screen space
				glm::vec3 u = glm::cross(
					glm::vec3(pts[2][0] - pts[0][0], pts[1][0] - pts[0][0], pts[0][0] - x),
					glm::vec3(pts[2][1] - pts[0][1], pts[1][1] - pts[0][1], pts[0][1] - y));

				glm::vec3 bc_screen = glm::vec3(1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);

				// to clip space
				glm::vec3 bc_clip = glm::vec3(bc_screen.x / pts[0][3], bc_screen.y / pts[1][3], bc_screen.z / pts[2][3]);
				// perspective deformation
				bc_clip = bc_clip / (bc_clip.x + bc_clip.y + bc_clip.z);

				double frag_depth = glm::dot(glm::vec3(clip_verts[0][2], clip_verts[1][2], clip_verts[2][2]), bc_clip);

				// discard in case of depth=isNaN due to grazing angles
				// TODO: might be solved earlier (per face) to improve processing performances
				if (std::isnan(frag_depth))
					continue;

				// clipping
				if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0 || frag_depth <= zbuffer[x + y * width])
					continue;

				glm::vec4 color;
				bool discard = shader.fragment(bc_clip, color);
				if (discard) continue;

				// write fragment
				zbuffer[x + y * width] = (float)frag_depth;
				for (glm::vec4::length_type c = 0; c < 4; ++c) {
					fbuffer[(x + y * width) * 4 + c] = (int)roundf(color[c]);
				}
			}
		}
	}
}

void RendererSw::clear(std::vector<uint8_t>& fbuffer, std::vector<float>& zbuffer) {
	// clear depth
	std::fill(zbuffer.begin(), zbuffer.end(), _clearDepth);
	// clear color
	char color[4];
	for (glm::vec4::length_type c = 0; c < 4; ++c)
		color[c] = (uint8_t)std::round(_clearColor[c]);
	for (size_t i = 0; i < fbuffer.size() / 4; ++i) {
		for (size_t c = 0; c < 4; ++c)
			fbuffer[i * 4 + c] = color[c];
	}
}

bool RendererSw::render(
	Model* model, const Image* map,
	std::vector<uint8_t>& fbuffer,
	std::vector<float>& zbuffer,
	const unsigned int width, const unsigned int height,
	const glm::vec3& viewDir, const glm::vec3& viewUp,
	const glm::vec3& bboxMin, const glm::vec3& bboxMax, bool useBBox
) {
	// fit ortho viewpoint to model bbox
	float ratio = (float)width / (float)height;
	glm::vec3 minPos, maxPos;
	if (useBBox) {
		minPos = bboxMin;
		maxPos = bboxMax;
	}
	else {
		computeBBox(model->vertices, minPos, maxPos);
	}
	glm::vec3 halfBox = (maxPos - minPos) * glm::vec3(0.5);
	glm::vec3 boxCtr = minPos + halfBox;
	float size = glm::length(halfBox);
	size = size + size / 100.0F; // add 1% so the model does not touch the image borders

	glm::mat4 mdl = glm::mat4(1.0);
	viewPosition = boxCtr + viewDir * size;
	glm::mat4 view = glm::lookAt(viewPosition, boxCtr, viewUp);
	viewPositionMV = modelView * glm::vec4(viewPosition, 1.0);
	// glob transfo to center view and fit OpenGL HW results.
	glm::mat4 glob =
		glm::translate(glm::mat4(1.0), glm::vec3(ratio * size, size, 0)) *
		glm::scale(glm::mat4(1.0), glm::vec3(1.0F, 1.0F, -1.0F));
	glm::mat4 proj = glm::ortho(-ratio * size, ratio * size, -size, size, 0.0F, 2.0F * size);

	modelView = view * mdl;
	mvp = proj * glob * modelView;
	normalMatrix = glm::inverseTranspose(modelView);
	viewPositionMVP = glm::vec3(proj * glob * glm::vec4(viewPositionMV, 1.0));

	if (_isLigthingEnabled) {
		lightColor = _lightColor;
		if (_isAutoLightPositionEnabled) {
			lightPosition = boxCtr + _lightAutoDir * size;
		}
		else {
			lightPosition = _lightPosition;
		}
		lightPositionMV = modelView * glm::vec4(lightPosition, 1.0);
		std::cout << "Light Pos = " << lightPositionMV.x << ", " << lightPositionMV.y << ", " << lightPositionMV.z << ", " << std::endl;
		//
		materialAmbient = _materialAmbient;
		materialDiffuse = _materialDiffuse;
		//
		if (!model->hasVertexNormals()) {
			std::cout << "Processing normals with \"noseams\" enabled..." << std::endl;
			model->computeVertexNormals(true, true);
		}
		else {
			std::cout << "Using pre-defined model normals." << std::endl;
		}
	}
	else {
		if (!model->hasTriangleNormals()) {
			std::cout << "Processing triangle normals " << std::endl;
			model->computeFaceNormals(true);
		}
	}

	isCullingEnabled = _isCullingEnabled;
	cwCulling = _cwCulling;

	viewport(0, 0, width, height);

	clear(fbuffer, zbuffer);

	if (model->uvcoords.size() != 0 && map != NULL && map->data != NULL) {
		if (_isLigthingEnabled && model->normals.size() != 0) {
			ShaderMapLight shader(model, map);
			rasterize(shader, fbuffer, width, height, zbuffer);
		}
		else {
			ShaderMap shader(model, map);
			rasterize(shader, fbuffer, width, height, zbuffer);
		}
	}
	else if (model->colors.size()) {
		ShaderCpv shader(model, NULL);
		rasterize(shader, fbuffer, width, height, zbuffer);
	}
	else {
		ShaderRed shader(model, NULL);
		rasterize(shader, fbuffer, width, height, zbuffer);
	}

	// optional level
	if (_isAutoLevelEnabled) {
		uint8_t maxDist = 0;
		// analyse
		for (size_t c = 0; c < fbuffer.size(); ++c) {
			if ((c + 1) % 4 == 0) continue;
			maxDist = std::max(maxDist, fbuffer[c]);
		}
		std::cout << "Auto Level MaxDist = " << (int)maxDist << std::endl;
		// level up the intensity 
		for (size_t c = 0; c < fbuffer.size(); ++c) {
			if ((c + 1) % 4 == 0) continue;
			fbuffer[c] = fbuffer[c] + (uint8_t)(255 - maxDist);
		}
	}

	return true;
}

bool RendererSw::render(
	Model* model, const Image* map, 
	const std::string& outputImage, const std::string& outputDepth,
	const unsigned int width, const unsigned int height,
	const glm::vec3& viewDir, const glm::vec3& viewUp,
	const glm::vec3& bboxMin, const glm::vec3& bboxMax, bool useBBox
) {

	// allocate depth buffer - will be init by RendererSw::clear
	std::vector<float> zbuffer(width * height);
	// allocate frame buffer - will be init by RendererSw::clear
	std::vector<uint8_t> fbuffer(width * height * 4);

	// render the model into memory buffers
	render( model, map,	fbuffer, zbuffer, width, height, 
		viewDir, viewUp, bboxMin, bboxMax, useBBox);

	if ( outputImage != "" ){
		// Write image Y-flipped because OpenGL
		stbi_write_png(outputImage.c_str(),
			width, height, 4,
			fbuffer.data() + (width * 4 * (height - 1)),
			-(int)width * 4);
	}

	if (outputDepth != "") {
		// Write depth splitted on RGBA
		stbi_write_png(outputDepth.c_str(),
			width, height, 4,
			(char*)zbuffer.data() + (width * 4 * (height - 1)),
			-(int)width * 4);
	}

	return true;
}