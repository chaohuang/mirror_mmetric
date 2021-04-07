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

#ifndef _MM_MODEL_H_
#define _MM_MODEL_H_

//
#include <set>
#include <vector>
#include <algorithm>
#include <functional>
// mathematics
#include <glm/vec3.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
// internal
#include "mmImage.h"

// 3D Model: mesh or point cloud
class Model {

public:
	std::string header;					// mostly for OBJ material
	std::vector<std::string> comments;	// mostly for PLY
	std::vector<float> vertices;		// vertex positions (x,y,z)
	std::vector<float> uvcoords;		// vertex uv coordinates (u,v)
	std::vector<float> normals;			// vertex normals (x,y,z)
	std::vector<float> colors;			// vertex colors (r,g,b)
	std::vector<float> faceNormals;		// per triangle normals (x,y,z)
	std::vector<int> triangles;			// triangle position indices
	std::vector<int> trianglesuv;		// triangle uv indices
	
	Model() {}

	bool isPointCloud(void) const { return hasVertices() && !hasTriangles(); }
	bool isMesh(void) const { return hasVertices() && !hasTriangles(); }

	bool hasTriangles(void) const { return ( triangles.size() != 0 ) && ( triangles.size() % 3 == 0 ); }
	bool hasVertices(void) const { return ( vertices.size() != 0 ) && ( vertices.size() % 3 == 0 ); }
	bool hasColors(void) const { return hasVertices() && colors.size() == vertices.size(); }
	
	bool hasUvCoords(void) const { 
		return uvcoords.size() != 0 && uvcoords.size() % 2 == 0 &&
			((trianglesuv.size() != 0 && trianglesuv.size() % 3 == 0) ||
			(hasVertices() && uvcoords.size() / 2 == vertices.size() / 3)); 
	}

	// return true if model has vertex normals
	bool hasNormals(void) const { return hasVertexNormals(); }
	// return true if model has vertex normals
	bool hasVertexNormals(void) const { return hasVertices() && normals.size() == vertices.size(); }
	// return true if model has face normals
	bool hasTriangleNormals(void) const { return hasTriangles() && faceNormals.size() == triangles.size(); }

	// return the number of triangles
	inline size_t getTriangleCount(void) const { return triangles.size() / 3; }

	// return the number of vertices
	inline size_t getPositionCount(void) const { return vertices.size() / 3;  }
	// return the number of colors 
	inline size_t getColorCount(void) const { return colors.size() / 3; }
	// return the number of uv coordinates
	inline size_t getUvCount(void) const { return uvcoords.size() / 2; }
	// return the number of normals
	inline size_t getNormalCount(void) const { return normals.size() / 3; }
	// return the number of face normals
	inline size_t getFaceNormalCount(void) const { return faceNormals.size() / 3; }

	// no sanity check (for performance reasons)
	inline glm::vec3 fetchPosition(const size_t triIdx, const size_t vertIdx) const {
		return glm::make_vec3(&vertices[triangles[triIdx * 3 + vertIdx] * 3]);
	}
	// no sanity check (for performance reasons)
	inline glm::vec3 fetchColor(const size_t triIdx, const size_t vertIdx) const {
		return glm::make_vec3(&colors[triangles[triIdx * 3 + vertIdx] * 3]);
	}
	// no sanity check (for performance reasons)
	inline glm::vec3 fetchNormal(const size_t triIdx, const size_t vertIdx) const {
		return glm::make_vec3(&normals[triangles[triIdx * 3 + vertIdx] * 3]);
	}
	// no sanity check (for performance reasons)
	inline glm::vec3 fetchNormal(const size_t normalIdx) const {
		return glm::make_vec3(&normals[normalIdx * 3]);
	}
	// no sanity check (for performance reasons)
	inline glm::vec2 fetchUv(const size_t triIdx, const size_t vertIdx) const {
		if (trianglesuv.size()) {
			return glm::make_vec2(&uvcoords[trianglesuv[triIdx * 3 + vertIdx] * 2]);
		}
		else {
			return glm::make_vec2(&uvcoords[triangles[triIdx * 3 + vertIdx] * 2]);
		}
	}
	// no sanity check (for performance reasons)
	inline glm::vec3 fetchFaceNormal(const size_t triIdx) const {
		return glm::make_vec3(&faceNormals[triIdx * 3]);
	}

	// no sanity check (for performance reasons)
	inline void fetchTriangleIndices(const size_t triIdx, int& i1, int& i2, int& i3) const {
		i1 = triangles[triIdx * 3 + 0];
		i2 = triangles[triIdx * 3 + 1];
		i3 = triangles[triIdx * 3 + 2];
	}

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
	inline void fetchTriangleNormals(const size_t triIdx, glm::vec3& n1, glm::vec3& n2, glm::vec3& n3) const {
		n1 = glm::make_vec3(&normals[triangles[triIdx * 3 + 0] * 3]);
		n2 = glm::make_vec3(&normals[triangles[triIdx * 3 + 1] * 3]);
		n3 = glm::make_vec3(&normals[triangles[triIdx * 3 + 2] * 3]);
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
	
	// normalize face and vertex normals if any
	// degenerate normals are replaced by (0,0,1)
	inline void normalizeNormals( void ) {

		// normalize vertex normals if any
		for (size_t i = 0; i < getNormalCount(); i++) {
			glm::vec3 normal = fetchNormal(i);
			normal = glm::normalize(normal);
			if (std::isnan(normal[0])) { 
				// use default z normal if invalid face
				normal = glm::vec3(0.0F, 0.0F, 1.0F);
			}
			for (glm::vec3::length_type c = 0; c < 3; c++) {
				normals[i * 3 + c] = normal[c];
			}
		}

		// normalze triangle normals if any
		for (size_t i = 0; i < getFaceNormalCount(); i++) {
			glm::vec3 normal = fetchFaceNormal(i);
			normal = glm::normalize(normal);
			if (std::isnan(normal[0])) { 
				// use default z normal if invalid face
				normal = glm::vec3(0.0F, 0.0F, 1.0F);
			}
			for (glm::vec3::length_type c = 0; c < 3; c++) {
				faceNormals[i * 3 + c] = normal[c];
			}
		}
	}

	// some normals might be invalid if faces are degenerate
	// normalize set to true will normalize and set invalid normals to 0,0,1
	inline void computeFaceNormals(bool normalize = true) {
		// allocate output
		faceNormals.resize(getTriangleCount() * 3);
		//
		for (size_t t = 0; t < getTriangleCount(); t++) {
			//
			const glm::vec3 v1 = fetchPosition(t, 0);
			const glm::vec3 v2 = fetchPosition(t, 1);
			const glm::vec3 v3 = fetchPosition(t, 2);
			// computes face normal
			glm::vec3 faceNormal;
			const glm::vec3 v12 = v2 - v1;
			const glm::vec3 v13 = v3 - v1;
			faceNormal = glm::cross(v12, v13);
			// write the result
			for (glm::vec3::length_type c = 0; c < 3; c++) {
				faceNormals[t * 3 + c] = faceNormal[c];
			}
		}
		// normalize if requested
		if (normalize) {
			normalizeNormals();
		}
	}

	// some normals might be invalid if faces are degenerate
	// normalize set to true will normalize and set invalid normals to 0,0,1
	// noSeams set to true will takes more time to compute but prevent seam effect if model has UV patches
	inline void computeVertexNormals(bool normalize = true, bool noSeams=true) {
		//
		if (!hasTriangleNormals()) {
			computeFaceNormals(false);
		}
		//
		normals.resize(vertices.size(), 0.0F);
		// map ( position -> map ( vertexIndex, faceNormal ))
		// used in no seams mode
		auto cmp = [](const glm::vec3& a, const glm::vec3& b) { 
			if (a.x != b.x) return a.x < b.x;
			if (a.y != b.y) return a.y < b.y;
			return a.z < b.z;
		};

		typedef std::map< int, glm::vec3> TmpVertNormals;
		typedef std::map<glm::vec3, TmpVertNormals,
			std::function<bool(const glm::vec3&, const glm::vec3&)>> TmpPosNormals;

		TmpPosNormals tmpNormals(cmp);
		
		//
		for (size_t t = 0; t < getTriangleCount(); t++) {

			int idx[3];
			fetchTriangleIndices(t, idx[0], idx[1], idx[2]);
			const glm::vec3 normal = fetchFaceNormal(t);

			if (noSeams) {
				glm::vec3 pos[3];
				fetchTriangleVertices(t, pos[0], pos[1], pos[2]);
				for (size_t i = 0; i < 3; i++) {
					// p1
					auto posIter = tmpNormals.find(pos[i]);
					bool found = false;
					if (posIter != tmpNormals.end()) {
						auto vertIter = posIter->second.find(idx[i]);
						if (vertIter != posIter->second.end()) {
							vertIter->second = vertIter->second + normal;
							found = true;
						}
					}
					if (!found) tmpNormals[pos[i]][idx[i]] = normal;
				}
			}
			else {
				for (size_t i = 0; i < 3; i++) {
					for (glm::vec3::length_type c = 0; c < 3; c++) {
						normals[idx[i] * 3 + c] += normal[c];
					}
				}
			}
		}

		if (noSeams) { // second pass in noseams mode
			TmpPosNormals::const_iterator posIter;
			TmpVertNormals::const_iterator vertIter;
			// iteratr the positions
			for (posIter = tmpNormals.begin(); posIter != tmpNormals.end(); posIter++) {
				// iterates the vertices that share the same position and cumulate normals
				glm::vec3 normal(0.0F, 0.0F, 0.0F);
				for (vertIter = posIter->second.begin(); vertIter != posIter->second.end(); vertIter++) {
					normal += vertIter->second;
				}
				// assign normal sum to all the indices of this position
				for (vertIter = posIter->second.begin(); vertIter != posIter->second.end(); vertIter++) {
					for (glm::vec3::length_type c = 0; c < 3; c++) {
						normals[vertIter->first * 3 + c] = normal[c];
					}
				}
			}
		}
		
		if (normalize) {
			normalizeNormals();
		}
	}

};

// structure for processings
class Vertex {
public:
	glm::vec3 pos;
	glm::vec2 uv;
	glm::vec3 col;
	glm::vec3 nrm;
	bool hasColor;
	bool hasUVCoord;
	bool hasNormal;

	Vertex() :
		pos(0.0, 0.0, 0.0),
		uv(0.0, 0.0),
		col(0.0, 0.0, 0.0),
		nrm(0.0, 0.0, 0.0),
		hasColor(false),
		hasUVCoord(false),
		hasNormal(false)
	{}

	inline bool operator==(Vertex& v) {
		if (pos != v.pos) return false;
		if (hasUVCoord && uv != v.uv) return false;
		if (hasColor && col != v.col) return false;
		if (hasNormal && nrm != v.nrm) return false;
		return true;
	}
};

// Compare two vertices, testing pos and/or Uv, and/or color and/or nromals
// can be used by std::sort, std::set and other std ordered paradigms
template <bool testPos, bool testUv, bool testCol, bool testNrm>
struct CompareVertex {
	bool operator()(const Vertex& a, const Vertex& b) const {
		
		if ( testPos ){
			if (a.pos.x != b.pos.x) return a.pos.x < b.pos.x;
			if (a.pos.y != b.pos.y) return a.pos.y < b.pos.y;
			if (a.pos.z != b.pos.z || (!testUv && !testCol && !testNrm))
				return a.pos.z < b.pos.z;
		}
		if ( testUv ) {
			if (a.uv.x != b.uv.x) return a.uv.x < b.uv.x;
			if (a.uv.y != b.uv.y || (!testCol && !testNrm))
				return a.uv.y < b.uv.y;
		}
		if ( testCol ) {
			if (a.col.x != b.col.x) return a.col.x < b.col.x;
			if (a.col.y != b.col.y) return a.col.y < b.col.y;
			if (a.col.z != b.col.z || !testNrm)
				return a.col.z < b.col.z;
		}
		if ( testNrm ){
			if (a.nrm.x != b.nrm.x) return a.nrm.x < b.nrm.x;
			if (a.nrm.y != b.nrm.y) return a.nrm.y < b.nrm.y;
			return a.nrm.z < b.nrm.z;
		}
	}
};

// Utility class to create Models using vertex 
// search for compact indexing and duplicate vertex removal
class ModelBuilder {
	
	// output model
	Model& output;
	// set of <sorted vertices, associated index>
	std::map<Vertex, size_t, CompareVertex<true, true, true, true> > vset;
public:
	// statistics
	size_t foundCount;

public:

	ModelBuilder( Model& output ):output(output),foundCount(0){}
		
	// method to construct point clouds,  with duplicate points removal
	// return index of the vertex
	inline size_t pushVertex(const Vertex& v)
	{
		// push only if not exist in vset
		auto searchIter = vset.find(v);
		if (searchIter != vset.end()) {
			foundCount++;
			return searchIter->second;
		}
		size_t newIndex = output.vertices.size() / 3;
		vset.insert(std::make_pair(v, newIndex));

		for (glm::vec3::length_type c = 0; c < 3; c++) {
			output.vertices.push_back(v.pos[c]);
		}

		if (v.hasNormal)
			for (glm::vec3::length_type c = 0; c < 3; c++)
				output.normals.push_back(v.nrm[c]);

		if (v.hasUVCoord) {
			for (glm::vec3::length_type c = 0; c < 2; c++)
				output.uvcoords.push_back(v.uv[c]);
		}

		if (v.hasColor) {
			for (glm::vec3::length_type c = 0; c < 3; c++)
				output.colors.push_back(v.col[c]);
		}

		return newIndex;
	}

	// method to construct point clouds,  with duplicate points removal
	// if texmap is valid and uv available, 
	// vertex color will be set with map texel using nearest or bilinear filter
	// otherwise per vertex color will be used if exists
	void pushVertex(const Vertex& v, const Image& tex_map, const bool bilinear) {

		if (v.hasUVCoord && tex_map.data != NULL) {

			Vertex tmp = v;

			// fetch the color from the map
			if (bilinear)
				texture2D_bilinear(tex_map, v.uv, tmp.col);
			else
				texture2D(tex_map, v.uv, tmp.col);

			tmp.hasUVCoord = false;
			tmp.hasColor = true;

			pushVertex(tmp);
			return;
		}

		pushVertex(v);
	}

	// method to create a mesh with duplicate vertex removal
	void pushTriangle(const Vertex& v1, const Vertex& v2, const Vertex& v3 ) {

		const auto i1 = pushVertex(v1);
		const auto i2 = pushVertex(v2);
		const auto i3 = pushVertex(v3);

		output.triangles.push_back((int)i1);
		output.triangles.push_back((int)i2);
		output.triangles.push_back((int)i3);
	}
};

// fetch a triangle, no sanity check for perf reasons
inline void fetchTriangle(const Model& model, size_t index, 
	bool hasUVCoords, bool hasColors, bool hasNormals,
	Vertex& v1, Vertex& v2, Vertex& v3) 
{
	model.fetchTriangleVertices(index, v1.pos, v2.pos, v3.pos);
	if (hasUVCoords) {
		v1.hasUVCoord = v2.hasUVCoord = v3.hasUVCoord = true;
		model.fetchTriangleUVs(index, v1.uv, v2.uv, v3.uv);
	}
	if (hasColors) {
		v1.hasColor = v2.hasColor = v3.hasColor= true;
		model.fetchTriangleColors(index, v1.col, v2.col, v3.col);
	}
	if (hasNormals) {
		v1.hasNormal = v2.hasNormal = v3.hasNormal = true;
		model.fetchTriangleNormals(index, v1.nrm, v2.nrm, v3.nrm);
	}
}

// reindex a mesh to use a single index table (model::triangles)
inline bool reindex(const Model& input, Model& output) {

	if (input.trianglesuv.size() != input.triangles.size())
		return false;

	// (vert index,uv index)->new index
	std::map<std::pair<size_t, size_t>, size_t> indexMap;

	// for each vertex of each face, 
	for (size_t v = 0; v < input.triangles.size(); ++v) {

		std::pair<size_t, size_t> idx(input.triangles[v], input.trianglesuv[v]);

		std::map<std::pair<size_t, size_t>, size_t>::iterator iter = indexMap.find(idx);

		// use existing index
		if (iter != indexMap.end()) {
			output.triangles.push_back((int)iter->second);
		}
		// create a new vertex
		else {
			size_t newIdx = output.vertices.size() / 3;
			output.triangles.push_back((int)newIdx);
			indexMap[idx] = newIdx;
			for (size_t i = 0; i < 3; i++) {
				output.vertices.push_back(input.vertices[idx.first * 3 + i]);
			}
			if (input.colors.size() == input.vertices.size()) {
				for (size_t i = 0; i < 3; i++) {
					output.colors.push_back(input.colors[idx.first * 3 + i]);
				}
			}
			if (input.normals.size() == input.vertices.size()) {
				for (size_t i = 0; i < 3; i++) {
					output.normals.push_back(input.normals[idx.first * 3 + i]);
				}
			}
			for (size_t i = 0; i < 2; i++) { 
				output.uvcoords.push_back(input.uvcoords[idx.second * 2 + i]);
			}
		}
	}

	return true;
}

// reorder a mesh so that vertices are sorted  
// faces enumeration allways start by the "smaller" vertex in the ordered set
// sorting = vertex will reindex and order the vertices only
// sorting = oriented will reindex, order the vertices and order face indexes 
//           to start with smallest index, while preserving orientation
// sorting = unoriented will reindex, order the vertices and sort face indexes 
//           from smallest to greatest index, thus not preserving orientation
inline bool reorder(const Model& input, std::string sorting, Model& output) 
{
	if (sorting != "vertex" && sorting != "oriented" && sorting != "unoriented")
		return false;

	const bool hasNormal = input.hasNormals();
	const bool hasUvCoord = input.hasUvCoords();
	const bool hasColor = input.hasColors();

	// A - first reconstruct the mesh so that we have a single index table 
	// and no duplicates vertices 
	Model reindexed;
	ModelBuilder builder(reindexed);
	for (size_t triIdx = 0; triIdx < input.triangles.size() / 3; ++triIdx) {
		Vertex v1, v2, v3;
		fetchTriangle(input, triIdx, hasUvCoord, hasColor, hasNormal, v1, v2, v3 );
		builder.pushTriangle(v1, v2, v3);
	}

	// B - second sort the vertices
	// vertex -> original index
	std::map<Vertex, int, CompareVertex<true, true, true, true> > sortedVertices;

	for (size_t triIdx = 0; triIdx < reindexed.triangles.size() / 3; ++triIdx ) {
		Vertex v1, v2, v3;
		fetchTriangle(reindexed, triIdx, hasUvCoord, hasColor, hasNormal, v1, v2, v3);
		sortedVertices[v1] = reindexed.triangles[triIdx * 3 + 0];
		sortedVertices[v2] = reindexed.triangles[triIdx * 3 + 1];
		sortedVertices[v3] = reindexed.triangles[triIdx * 3 + 2];
	}

	// create output vertices
	std::vector<int> vertexTranslate;
	vertexTranslate.resize(sortedVertices.size());

	for (auto iter = sortedVertices.begin(); iter != sortedVertices.end(); ++iter) {
		
		for (glm::vec3::length_type c = 0; c < 3; c++)
			output.vertices.push_back(iter->first.pos[c]);
		
		if (reindexed.normals.size())
			for (glm::vec3::length_type c = 0; c < 3; c++)
				output.normals.push_back(iter->first.nrm[c]);

		if (reindexed.uvcoords.size())
			for (glm::vec3::length_type c = 0; c < 2; c++)
				output.uvcoords.push_back(iter->first.uv[c]);

		if (reindexed.colors.size())
			for (glm::vec3::length_type c = 0; c < 3; c++)
				output.colors.push_back(iter->first.col[c]);

		// associate original index to new index
		vertexTranslate[iter->second] = (int) (output.vertices.size() / 3) - 1;
	}

	// create output triangles
	output.triangles.resize(reindexed.triangles.size());

	for (size_t vertIdx = 0; vertIdx < output.triangles.size(); ++vertIdx) {
		output.triangles[vertIdx] = vertexTranslate[reindexed.triangles[vertIdx]];
	}

	if (sorting == "vertex") return true;

	// C - now we have a new mesh, with unique and sorted vertices
	// depending on parameters we reorder the triangle vertices
	// in order to be "canonical"
	// i.e. if two meshes mathematically identical were not having 
	// same order of vertices or faces, they will after this step
	
	bool oriented = (sorting == "oriented");

	for (size_t triIdx = 0; triIdx < output.triangles.size() / 3; ++triIdx) {

		// shift the triangle vertices so that first vertex has smaller index
		// hence has smaller vertex since they are sorted. preserve orientation
		if (oriented) { // preserve original orientation
			if (output.triangles[triIdx * 3 + 1] < output.triangles[triIdx * 3 + 0] &&
				output.triangles[triIdx * 3 + 1] < output.triangles[triIdx * 3 + 2]) {
				// output.triangles[triIdx * 3 + 1] the smallest index
				// rotate left one step
				std::swap(output.triangles[triIdx * 3 + 0], output.triangles[triIdx * 3 + 1]);
				std::swap(output.triangles[triIdx * 3 + 2], output.triangles[triIdx * 3 + 1]);
			}
			else if (output.triangles[triIdx * 3 + 2] < output.triangles[triIdx * 3 + 0] &&
				output.triangles[triIdx * 3 + 2] < output.triangles[triIdx * 3 + 1]) {
				// output.triangles[triIdx * 3 + 2] the smallest index
				// rotate left two steps
				std::swap(output.triangles[triIdx * 3 + 0], output.triangles[triIdx * 3 + 2]);
				std::swap(output.triangles[triIdx * 3 + 1], output.triangles[triIdx * 3 + 2]);
			}
		}
		else { // stronger reorder, does not preserve orientation
			std::sort(&output.triangles[triIdx * 3 + 0], &output.triangles[triIdx * 3 + 2]+1);
		}

	}
	return true;
}

#endif