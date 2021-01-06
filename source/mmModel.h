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

#ifndef _MM_MODEL_H_
#define _MM_MODEL_H_

//
#include <vector>
// mathematics
#include <glm/vec3.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

// 3D Model: mesh or point cloud
class Model {

public:
	std::string header;					// mostly for OBJ material
	std::vector<std::string> comments;	// mostly for PLY
	std::vector<float> vertices;
	std::vector<float> uvcoords;
	std::vector<float> normals;
	std::vector<float> colors;
	std::vector<int> triangles;
	std::vector<int> trianglesuv;

	Model() {}

	// no sanity check (for performance reasons)
	inline void fetchTriangleVertices(const size_t triIdx, glm::vec3& v1, glm::vec3& v2, glm::vec3& v3) const {
		v1 = glm::make_vec3(&vertices[triangles[triIdx * 3 + 0] * 3]);
		v2 = glm::make_vec3(&vertices[triangles[triIdx * 3 + 1] * 3]);
		v3 = glm::make_vec3(&vertices[triangles[triIdx * 3 + 2] * 3]);
	}

	// no sanity check (for performance reasons)
	inline void fetchTriangleColors(const size_t triIdx, glm::vec3& c1, glm::vec3& c2, glm::vec3& c3) const {
		c1 = glm::make_vec3(&colors[triangles[triIdx * 3 + 0] * 3]);
		c2 = glm::make_vec3(&colors[triangles[triIdx * 3 + 1] * 3]);
		c3 = glm::make_vec3(&colors[triangles[triIdx * 3 + 2] * 3]);
	}

	// no sanity check (for performance reasons)
	inline void fetchTriangleUVs(const size_t triIdx, glm::vec2& uv1, glm::vec2& uv2, glm::vec2& uv3) const {
		if (trianglesuv.size()) {
			uv1 = glm::make_vec2(&uvcoords[trianglesuv[triIdx * 3 + 0] * 2]);
			uv2 = glm::make_vec2(&uvcoords[trianglesuv[triIdx * 3 + 1] * 2]);
			uv3 = glm::make_vec2(&uvcoords[trianglesuv[triIdx * 3 + 2] * 2]);
		}
		else {
			uv1 = glm::make_vec2(&uvcoords[triangles[triIdx * 3 + 0] * 2]);
			uv2 = glm::make_vec2(&uvcoords[triangles[triIdx * 3 + 1] * 2]);
			uv3 = glm::make_vec2(&uvcoords[triangles[triIdx * 3 + 2] * 2]);
		}
	}

};

#endif