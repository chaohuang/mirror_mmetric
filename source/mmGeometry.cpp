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

//
#include <vector>
#include "glm/glm.hpp"
//
#include "mmGeometry.h"

// Compute barycentric coordinates (u, v, w)~res(x,y,z) for
// point p with respect to triangle (a, b, c)
bool getBarycentric(glm::vec2 p, glm::vec2 a, glm::vec2 b, glm::vec2 c, glm::vec3& res)
{
    glm::vec2 v0 = b - a, v1 = c - a, v2 = p - a;
    float den = v0.x * v1.y - v1.x * v0.y;
    float u = (v2.x * v1.y - v1.x * v2.y) / den;
    float v = (v0.x * v2.y - v2.x * v0.y) / den;
    float w = 1.0f - u - v;
    res.x = u; res.y = v; res.z = w;
    if (0 <= u && u <= 1 && 0 <= v && v <= 1 && u + v <= 1)
        return true;
    else
        return false;
}

// return true if there is an intersection
// and res will contain t (the parameter on the ray to intersection)
// and u,v the barycentric of the intersection on the triangle surface
bool evalRayTriangle(
    const glm::vec3& rayOrigin, const glm::vec3& rayDirection,
    const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2, 
    glm::vec3& res, float epsilon)
{

    glm::vec3 tvec, pvec, qvec;
    float det, inv_det, t, u, v;

    /* find vectors for two edges sharing vert0 */
    glm::vec3 edge1(v1 - v0);
    glm::vec3 edge2(v2 - v0);

    /* begin calculating determinant - also used to calculate U parameter */
    pvec = glm::cross(rayDirection, edge2);

    /* if determinant is near zero, ray lies in plane of triangle */
    det = glm::dot( edge1, pvec);

    /* the non-culling branch */
    if (det > -epsilon && det < epsilon)
        return false;

    inv_det = 1.0f / det;

    /* calculate distance from vert0 to ray origin */
    tvec = rayOrigin - v0;

    /* calculate U parameter and test bounds */
    u = glm::dot( tvec, pvec ) * inv_det;

    if (u < 0.0f || u > 1.0f)
        return false;

    /* prepare to test V parameter */
    qvec = glm::cross( tvec, edge1 );

    /* calculate V parameter and test bounds */
    v = glm::dot( rayDirection, qvec ) * inv_det;

    if (v < 0.0f || u + v > 1.0f)
        return false;

    /* calculate t, ray intersects triangle */
    t = glm::dot( edge2, qvec ) * inv_det;

    // push the result
    res.x = t;
    res.y = u;
    res.z = v;

    return true;
}
