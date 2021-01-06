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

#ifndef _MM_GEOMETRY_H_
#define _MM_GEOMETRY_H_

//
#include <vector>
#include "glm/glm.hpp"

// minimum precision parameter do not change
#define  ZERO_TOLERANCE 1e-06F

// computes the position of a point from triangle (v0,v1,v2) and barycentric coordinates (u,v)
inline void triangleInterpolation(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2, float u, float v, glm::vec3& p)
{
    p = v0 * (1.0f - u - v) + v1 * u + v2 * v;
}

// computes the normal to the triangle
inline void triangleNormal(const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3, glm::vec3& n)
{
    n = glm::normalize(glm::cross(v2 - v1, v3 - v1));
}

inline double triangleArea(const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3)
{
	return 0.5 * glm::length( glm::cross(v2 - v1, v3 - v1) );
}

// computes the bounding box of the triangle
inline void triangleBBox(const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3, glm::vec3& minPos, glm::vec3& maxPos)
{
	minPos = { FLT_MAX, FLT_MAX, FLT_MAX };
	maxPos = { -FLT_MAX, -FLT_MAX, -FLT_MAX };
	minPos = glm::min(v1, minPos);
	minPos = glm::min(v2, minPos);
	minPos = glm::min(v3, minPos);
	maxPos = glm::max(v1, maxPos);
	maxPos = glm::max(v2, maxPos);
	maxPos = glm::max(v3, maxPos);
}

// if reset is false the minPos and maxPos will not be reset before processing
// this permits to invoke the function on several point set and obtain the global box
inline void computeBBox(const std::vector<float>& vertices, glm::vec3& minPos, glm::vec3& maxPos, bool reset=true) {
	if (reset) {
		minPos = { FLT_MAX, FLT_MAX, FLT_MAX };
		maxPos = { -FLT_MAX, -FLT_MAX, -FLT_MAX };
	}
	for (size_t i = 0; i < vertices.size() / 3; i++) {
		for (glm::vec3::length_type j = 0; j < 3; j++) {
			minPos[j] = std::fmin(vertices[i * 3 + j], minPos[j]);
			maxPos[j] = std::fmax(vertices[i * 3 + j], maxPos[j]);
		}
	}
}

// transform the box into a cube bbox fitting original
inline void toCubicalBBox(glm::vec3& minPos, glm::vec3& maxPos) {
	minPos.x = minPos.y = minPos.z = std::min(std::min(minPos.x, minPos.y), minPos.z);
	maxPos.x = maxPos.y = maxPos.z = std::max(std::max(maxPos.x, maxPos.y), minPos.z);
}

// Compute barycentric coordinates (u, v, w)~res(x,y,z) for
// point p with respect to triangle (a, b, c)
// return true if point is inside the triangle
bool getBarycentric(glm::vec2 p, glm::vec2 a, glm::vec2 b, glm::vec2 c, glm::vec3& res);

// return true if there is an intersection
// and res will contain t (the parameter on the ray to intersection)
// and u,v the barycentric of the intersection on the triangle surface
bool evalRayTriangle(
    const glm::vec3& rayOrigin, const glm::vec3& rayDirection,
    const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2,
    glm::vec3& res, float epsilon = ZERO_TOLERANCE);

#endif
