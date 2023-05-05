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

#include <iostream>
#include <algorithm>
#include <fstream>
#include <unordered_map>
#include <time.h>
#include <math.h>
#include <string>
#include <vector>
// mathematics
#include <glm/vec3.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/transform.hpp>
// argument parsing
#include <cxxopts.hpp>

//
#include "dmetric/source/pcc_processing.hpp"
#include "pcqm/pcqm.h"
// internal headers
#include "mmIO.h"
#include "mmModel.h"
#include "mmImage.h"
#include "mmSample.h"
#include "mmGeometry.h"
#include "mmColor.h"
#include "mmRendererSw.h"
#include "mmRendererHw.h"
#include "mmStatistics.h"
#include "mmCompare.h"

// "implementation" done in mmRendererHW
#include <stb_image_write.h>

using namespace mm;

// A 1 2 3
// B 1 2 3
//    / /
//   2 3 1
//    / /
//   3 1 2
// twosided (we turn in other direction)
// B 1 3 2
//    / /
//   3 2 1
//    / /
//   2 1 3

inline bool areTrianglesEqual( bool              unoriented,
                               const mm::Vertex& vA1,
                               const mm::Vertex& vA2,
                               const mm::Vertex& vA3,
                               const mm::Vertex& vB1,
                               const mm::Vertex& vB2,
                               const mm::Vertex& vB3 ) {
  return ( vA1 == vB1 && vA2 == vB2 && vA3 == vB3 ) || ( vA1 == vB2 && vA2 == vB3 && vA3 == vB1 )
         || ( vA1 == vB3 && vA2 == vB1 && vA3 == vB2 )
         || ( unoriented
              && ( ( vA1 == vB1 && vA2 == vB3 && vA3 == vB2 ) || ( vA1 == vB3 && vA2 == vB2 && vA3 == vB1 )
                   || ( vA1 == vB2 && vA2 == vB1 && vA3 == vB3 ) ) );
}

Compare::Compare() : _hwRendererInitialized( false ) {}
Compare::~Compare() {
  if ( _hwRendererInitialized ) { _hwRenderer.shutdown(); }
}

int Compare::equ( const mm::Model& inputA,
                  const mm::Model& inputB,
                  const mm::Image& mapA,
                  const mm::Image& mapB,
                  float            epsilon,
                  bool             earlyReturn,
                  bool             unoriented,
                  mm::Model&       outputA,
                  mm::Model&       outputB ) {
  // we test the maps
  if ( mapA.data != NULL || mapB.data != NULL ) {
    if ( mapA.data == NULL ) {
      std::cout << "texture maps are not equal: mapA is null" << std::endl;
    } else if ( mapB.data == NULL ) {
      std::cout << "texture maps are not equal: mapB is null" << std::endl;
    } else {
      if ( mapA.width != mapB.width || mapA.height != mapB.height ) {
        std::cout << "texture maps are not equal: dimensions are not equal" << std::endl;
      } else {
        size_t diffs = 0;
        for ( size_t row = 0; row < mapA.height; ++row ) {
          for ( size_t col = 0; col < mapA.width; ++col ) {
            glm::vec3 colA, colB;
            mapA.fetchRGB( col, row, colA );
            mapB.fetchRGB( col, row, colB );
            if ( colA != colB ) ++diffs;
          }
        }
        if ( diffs != 0 ) {
          std::cout << "texture maps are not equal: " << diffs << "pixel differences" << std::endl;
        } else {
          std::cout << "texture maps are equal" << std::endl;
        }
      }
    }
  } else {
    std::cout << "skipping texture maps comparison" << std::endl;
  }

  if ( inputA.triangles.size() != inputB.triangles.size() ) {
    std::cout << "meshes are not equal, number of triangles are different " << inputA.triangles.size() / 3 << " vs "
              << inputB.triangles.size() / 3 << std::endl;
    return true;
  }

  // mesh mode
  if ( inputA.triangles.size() != 0 ) {
    // prepare a store for face status
    // doundInB[true] if the nth face of B matches one face of A
    std::vector<bool> foundInB( inputB.getTriangleCount(), false );

    const bool hasColors   = inputA.hasColors() && inputB.hasColors();
    const bool hasUvCoords = inputA.hasUvCoords() && inputB.hasUvCoords();
    const bool hasNormals  = inputA.hasNormals() && inputB.hasNormals();

    size_t diffs = 0;  // count the differences if no earlyReturn option

    // iterate over triangles of modelA
    for ( size_t triIdx = 0; triIdx < inputA.triangles.size() / 3; triIdx++ ) {
      mm::Vertex v1, v2, v3;
      mm::fetchTriangle( inputA, triIdx, hasUvCoords, hasColors, hasNormals, v1, v2, v3 );
      // search over modelB riangles that are not already matched
      bool   found   = false;
      size_t triIdxB = 0;
      while ( !found && triIdxB != inputB.triangles.size() / 3 ) {
        if ( foundInB[triIdxB] ) {
          ++triIdxB;
          continue;
        }
        mm::Vertex vB1, vB2, vB3;
        mm::fetchTriangle( inputB, triIdxB, hasUvCoords, hasColors, hasNormals, vB1, vB2, vB3 );
        if ( areTrianglesEqual( unoriented, v1, v2, v3, vB1, vB2, vB3 ) ) {
          found             = true;
          foundInB[triIdxB] = true;
        }
        ++triIdxB;
      }
      if ( !found ) {
        if ( earlyReturn ) {
          std::cout << "meshes are not equal, early return." << std::endl;
          std::cout << "triangle number " << triIdx << " from A has no equivalent in B" << std::endl;
          return true;
        }
        ++diffs;
      }
    }

    if ( diffs == 0 ) {
      std::cout << "meshes are equal" << std::endl;
    } else {
      std::cout << "meshes are not equal, " << diffs << " different triangles" << std::endl;
      // provide more details on what is different at additional computation cost (request from Sony team)
      size_t vertDiffs = std::abs( (long long)( inputA.getPositionCount() - inputB.getPositionCount() ) );
      for ( size_t index = 0; index < std::min( inputA.getPositionCount(), inputB.getPositionCount() ); ++index ) {
        if ( inputA.fetchPosition( index ) != inputB.fetchPosition( index ) ) ++vertDiffs;
      }
      std::cout << "Position differences: " << vertDiffs << std::endl;
      //
      size_t uvDiffs = std::abs( (long long)( inputA.getUvCount() - inputB.getUvCount() ) );
      for ( size_t index = 0; index < std::min( inputA.getUvCount(), inputB.getUvCount() ); ++index ) {
        if ( inputA.fetchUv( index ) != inputB.fetchUv( index ) ) ++uvDiffs;
      }
      std::cout << "UV coords differences: " << uvDiffs << std::endl;
      //
      size_t colorDiffs = std::abs( (long long)( inputA.getColorCount() - inputB.getColorCount() ) );
      for ( size_t index = 0; index < std::min( inputA.getColorCount(), inputB.getColorCount() ); ++index ) {
        if ( inputA.fetchColor( index ) != inputB.fetchColor( index ) ) ++colorDiffs;
      }
      std::cout << "Colors differences: " << colorDiffs << std::endl;
      //
      size_t normalDiffs = std::abs( (long long)( inputA.getNormalCount() - inputB.getNormalCount() ) );
      for ( size_t index = 0; index < std::min( inputA.getNormalCount(), inputB.getNormalCount() ); ++index ) {
        if ( inputA.fetchNormal( index ) != inputB.fetchNormal( index ) ) ++normalDiffs;
      }
      std::cout << "Normals differences: " << normalDiffs << std::endl;
    }
    return true;
  }
  // Point cloud mode, sort vertices then compare
  else {
    // allocate room for the results
    outputA.vertices.resize( inputA.vertices.size() );
    outputB.vertices.resize( inputB.vertices.size() );

    // prepare outputA
    std::vector<mm::Vertex> positions;
    mm::Vertex              vertex;
    for ( int i = 0; i < inputA.vertices.size() / 3; i++ ) {
      vertex.pos = glm::vec3( inputA.vertices[i * 3 + 0], inputA.vertices[i * 3 + 1], inputA.vertices[i * 3 + 2] );
      positions.push_back( vertex );
    }
    std::sort( positions.begin(), positions.end(), mm::CompareVertex<true, false, false, false>() );

    for ( int i = 0; i < positions.size(); i++ ) {
      outputA.vertices[i * 3 + 0] = positions[i].pos.x;
      outputA.vertices[i * 3 + 1] = positions[i].pos.y;
      outputA.vertices[i * 3 + 2] = positions[i].pos.z;
    }

    // prepare outputB
    positions.clear();
    for ( int i = 0; i < inputB.vertices.size() / 3; i++ ) {
      vertex.pos = glm::vec3( inputB.vertices[i * 3 + 0], inputB.vertices[i * 3 + 1], inputB.vertices[i * 3 + 2] );
      positions.push_back( vertex );
    }
    std::sort( positions.begin(), positions.end(), mm::CompareVertex<true, false, false, false>() );

    for ( int i = 0; i < positions.size(); i++ ) {
      outputB.vertices[i * 3 + 0] = positions[i].pos.x;
      outputB.vertices[i * 3 + 1] = positions[i].pos.y;
      outputB.vertices[i * 3 + 2] = positions[i].pos.z;
    }

    // now compare the results
    if ( epsilon == 0 ) {
      if ( outputB.vertices == outputA.vertices ) {
        std::cout << "model vertices are equals" << std::endl;
        return true;
      } else {
        std::cout << "model vertices are not equals" << std::endl;
        return true;
      }
    } else {
      if ( outputA.vertices.size() != outputB.vertices.size() ) {
        std::cout << "model vertices are not equals" << std::endl;
        return true;
      }
      size_t count = 0;
      for ( size_t i = 0; i < outputA.vertices.size() / 3; i++ ) {
        glm::vec3 A = glm::make_vec3( &outputA.vertices[i * 3] );
        glm::vec3 B = glm::make_vec3( &outputB.vertices[i * 3] );
        if ( glm::length( A - B ) >= epsilon ) { ++count; }
      }
      if ( count == 0 ) {
        std::cout << "model vertices are equals" << std::endl;
      } else {
        std::cout << "model vertices are not equals, found " << count << " differences" << std::endl;
      }
      return true;
    }
  }
}

int Compare::topo( const mm::Model&   modelA,
                   const mm::Model&   modelB,
                   const std::string& faceMapFilename,
                   const std::string& vertexMapFilename ) {
  // 1 - Test if number of triangles of output matches input number of triangles
  if ( modelA.getTriangleCount() != modelB.getTriangleCount() ) {
    std::cout << "Topologies are different: number of triangles differs (A=" << modelA.getTriangleCount()
              << ",B=" << modelB.getTriangleCount() << ").";
    return false;
  }

  // 2 - parse the face map and test bijection

  // where to store the face map,
  // faceMap[dest face index] = source face index
  std::vector<size_t> faceMap( modelA.getTriangleCount(), 0 );
  // did we already set one association for the given index
  std::vector<bool> visitedFace( modelA.getTriangleCount(), false );
  std::ifstream     faceFile;
  faceFile.open( faceMapFilename.c_str(), std::ios::in );
  if ( !faceFile ) {
    std::cerr << "Error: can't open topology face mapping file " << faceMapFilename << std::endl;
    return false;
  }
  // file parsing
  size_t      lineNo = 0;
  std::string line;
  std::getline( faceFile, line );
  while ( faceFile ) {
    // parse the line
    std::istringstream in( line );
    size_t             faces[2];
    for ( size_t index = 0; index < 2; ++index ) {
      if ( !( in >> faces[index] ) ) {
        std::cerr << "Error: " << faceMapFilename << ":" << lineNo << " missing face number " << index << std::endl;
        return false;
      }
      if ( faces[index] >= modelA.getTriangleCount() ) {
        std::cerr << "Error: " << faceMapFilename << ":" << lineNo << " face index out of range (faces[" << index
                  << "]=" << faces[index] << ") >= (modelA.getTriangleCount()=" << modelA.getTriangleCount() << ")"
                  << std::endl;
        return false;
      }
    }
    if ( visitedFace[faces[0]] ) {
      std::cerr << "Error: " << faceMapFilename << ":" << lineNo << " modelB face " << faces[0]
                << " already associated with modelA face " << faceMap[faces[0]] << std::endl;
      return false;
    }
    visitedFace[faces[0]] = true;
    faceMap[faces[0]]     = faces[1];
    // read next line
    std::getline( faceFile, line );
    lineNo++;
  }
  faceFile.close();

  // we already know that modelA and modelA tri count are equal
  // and that association map did not contain out of range indices
  // and that each association was unique
  // so just need to check that every entry has an association
  bool bijective = true;
  for ( size_t index = 0; index < visitedFace.size(); ++index ) { bijective = bijective && visitedFace[index] == 1; }
  if ( !bijective ) {
    std::cout << "Topologies are different: topology face map is not bijective." << std::endl;
    return false;
  }

  // 3 - parse the vertex map and test bijection

  // where to store the face map,
  // faceMap[dest face index] = source face index
  std::vector<size_t> vertexMap( modelA.getPositionCount(), 0 );
  // did we already set one association for the given index
  std::vector<bool> visitedVertex( modelA.getPositionCount(), false );
  std::ifstream     vertexFile;
  vertexFile.open( vertexMapFilename.c_str(), std::ios::in );
  if ( !vertexFile ) {
    std::cerr << "Error: can't open topology vertex mapping file " << vertexMapFilename << std::endl;
    return false;
  }
  // file parsing
  lineNo = 0;
  std::getline( vertexFile, line );
  while ( vertexFile ) {
    // parse the line
    std::istringstream in( line );
    size_t             vertex[2];
    for ( size_t index = 0; index < 2; ++index ) {
      if ( !( in >> vertex[index] ) ) {
        std::cerr << "Error: " << vertexMapFilename << ":" << lineNo << " missing vertex number " << index << std::endl;
        return false;
      }
      if ( vertex[index] >= modelA.getPositionCount() ) {
        std::cerr << "Error: " << vertexMapFilename << ":" << lineNo << " vertex index out of range (vertex[" << index
                  << "]=" << vertex[index] << ") >= (modelA.getPositionCount()=" << modelA.getPositionCount() << ")"
                  << std::endl;
        return false;
      }
    }
    if ( visitedVertex[vertex[0]] ) {
      std::cerr << "Error: " << vertexMapFilename << ":" << lineNo << " modelB vertex " << vertex[0]
                << " already associated with modelA vertex " << vertexMap[vertex[0]] << std::endl;
      return false;
    }
    visitedVertex[vertex[0]] = true;
    vertexMap[vertex[0]]     = vertex[1];
    // read next line
    std::getline( vertexFile, line );
    lineNo++;
  }
  vertexFile.close();

  // we already know that modelA and modelA tri count are equal
  // and that association map did not contain out of range indices
  // and that each association was unique
  // so just need to check that every entry has an association
  bijective = true;
  for ( size_t index = 0; index < visitedVertex.size(); ++index ) {
    bijective = bijective && visitedVertex[index] == 1;
  }
  if ( !bijective ) {
    std::cout << "Topologies are different: topology vertex map is not bijective." << std::endl;
    return false;
  }

  // 3 - Test if each output triangle respects the orientation of its associated input triangle
  // recall that modelA and modelB trianglecount are equals
  // we want the order of the vertices' indices to be the same or translated
  // that is ABC is equivalente to BCA and CAB, but not with ACB, BAC or CBA
  for ( size_t triIdx = 0; triIdx < modelB.getTriangleCount(); ++triIdx ) {
    int A1, A2, A3, B1, B2, B3;
    modelA.fetchTriangleIndices( faceMap[triIdx], A1, A2, A3 );
    modelB.fetchTriangleIndices( triIdx, B1, B2, B3 );
    size_t m1 = vertexMap[B1];
    size_t m2 = vertexMap[B2];
    size_t m3 = vertexMap[B3];
    if ( !( ( ( A1 == m1 ) && ( A2 == m2 ) && ( A3 == m3 ) ) || ( ( A1 == m2 ) && ( A2 == m3 ) && ( A3 == m1 ) )
            || ( ( A1 == m3 ) && ( A2 == m1 ) && ( A3 == m2 ) ) ) ) {
      std::cout << "Topologies are different: orientations are not preserved " << std::endl;
      std::cout << "modelA->Triangle[" << faceMap[triIdx] << "] = [" << A1 << "," << A2 << "," << A3 << ")]."
                << std::endl;
      std::cout << "modelB(with mapped indices from modelA)->Triangle[" << triIdx << "] = [" << m1 << "," << m2 << ","
                << m3 << ")]." << std::endl;
      return false;
    };
  }

  std::cout << "Topologies are matching." << std::endl;

  return true;
}

void sampleIfNeeded( const mm::Model& input, const mm::Image& map, mm::Model& output ) {
  if ( input.triangles.size() != 0 ) {
    // first reorder the model to prevent small variations
    // when having two similar topologies but not same orders of enumeration
    mm::Model reordered;
    reorder( input, std::string( "oriented" ), reordered );

    // then use face subdivision without map citerion and area threshold of 2.0
    mm::Sample::meshToPcDiv( reordered, output, map, 2.0, false, true, false );

  } else {
    output = input;  //  pass through
  }
}

// code  from PCC_error (prevents pcc_error library modification.
int removeDuplicatePoints( PccPointCloud& pc, int dropDuplicates, int neighborsProc, const bool verbose = true ) {
  // sort the point cloud
  std::vector<size_t> indices;
  indices.resize( pc.size );
  for ( size_t i = 0; i < pc.size; i++ ) { indices[i] = i; }
  std::sort( indices.begin(), indices.end(), [=, &pc]( const size_t& a, const size_t& b ) -> bool {
    return pc.xyz.p[a] == pc.xyz.p[b] ? a < b : pc.xyz.p[a] < pc.xyz.p[b];
  } );
  auto xyz    = pc.xyz.p;
  auto rgb    = pc.rgb.c;
  auto normal = pc.normal.n;
  auto lidar  = pc.lidar.reflectance;
  if ( pc.bXyz ) {
    for ( size_t i = 0; i < pc.size; i++ ) { pc.xyz.p[i] = xyz[indices[i]]; }
  }
  if ( pc.bRgb ) {
    for ( size_t i = 0; i < pc.size; i++ ) { pc.rgb.c[i] = rgb[indices[i]]; }
  }
  if ( pc.bNormal ) {
    for ( size_t i = 0; i < pc.size; i++ ) { pc.normal.n[i] = normal[indices[i]]; }
  }
  if ( pc.bLidar ) {
    for ( size_t i = 0; i < pc.size; i++ ) { pc.lidar.reflectance[i] = lidar[indices[i]]; }
  }
  // Find runs of identical point positions
  bool errorFind = false;
  for ( auto it_seq = pc.begin(); it_seq != pc.end(); ++it_seq ) {
    auto it_next = it_seq + 1;
    if ( it_next != pc.end() ) {
      if ( *it_next < *it_seq && !errorFind ) {
        printf( "WARNING: something went wrong between %zu and %zu \n", it_seq.idx, it_next.idx );
        errorFind = true;
      }
    }
  }
  if ( errorFind ) { printf( "WARNING: something went wrong in duplicate point sort process\n" ); }
  // Find runs of identical point positions
  for ( auto it_seq = pc.begin(); it_seq != pc.end(); ) {
    it_seq = std::adjacent_find( it_seq, pc.end() );
    if ( it_seq == pc.end() || it_seq.idx >= pc.size) break;

    // accumulators for averaging attribute values
    long cattr[3]{};  // sum colors.
    long lattr{};     // sum lidar.
    float nattr[3]{};  // sum normals.

    // iterate over the duplicate points, accumulating values
    int  count = 0;
    auto it    = it_seq;
    for ( ; ( it.idx < pc.size ) && ( *it == *it_seq ); ++it ) {
      if ( it.idx < pc.size ) {
        count++;
        size_t idx = it.idx;
        if ( pc.bRgb ) {
          cattr[0] += pc.rgb.c[idx][0];
          cattr[1] += pc.rgb.c[idx][1];
          cattr[2] += pc.rgb.c[idx][2];
        }
        if ( pc.bLidar ) lattr += pc.lidar.reflectance[idx];
        if ( pc.bNormal ) {
            nattr[0] += pc.normal.n[idx][0];
            nattr[1] += pc.normal.n[idx][1];
            nattr[2] += pc.normal.n[idx][2];
        }
      }
    }

    size_t first_idx = it_seq.idx;
    it_seq           = it;

    // averaging case only
    if ( dropDuplicates != 2 ) continue;

    if ( pc.bRgb ) {
      pc.rgb.c[first_idx][0] = (unsigned char)( cattr[0] / count );
      pc.rgb.c[first_idx][1] = (unsigned char)( cattr[1] / count );
      pc.rgb.c[first_idx][2] = (unsigned char)( cattr[2] / count );
    }

    if ( neighborsProc == 2 ) pc.xyz.nbdup[first_idx] = count;

    if ( pc.bLidar ) pc.lidar.reflectance[first_idx] = (unsigned short)( lattr / count );

    if ( pc.bNormal ) {
        pc.normal.n[first_idx][0] = (float)(nattr[0] / (float)count);
        pc.normal.n[first_idx][1] = (float)(nattr[1] / (float)count);
        pc.normal.n[first_idx][2] = (float)(nattr[2] / (float)count);
    }
  }

  int duplicatesFound = 0;
  if ( dropDuplicates != 0 ) {
    auto last       = std::unique( pc.begin(), pc.end() );
    duplicatesFound = (int)std::distance( last, pc.end() );

    pc.size -= duplicatesFound;
    pc.xyz.p.resize( pc.size );
    pc.xyz.nbdup.resize( pc.size );
    if ( pc.bNormal ) pc.normal.n.resize( pc.size );
    if ( pc.bRgb ) pc.rgb.c.resize( pc.size );
    if ( pc.bLidar ) pc.lidar.reflectance.resize( pc.size );
  }

  if ( verbose && duplicatesFound > 0 ) {
    switch ( dropDuplicates ) {
    case 0: printf( "WARNING: %d points with same coordinates found in \n", duplicatesFound ); break;
    case 1: printf( "WARNING: %d points with same coordinates found and dropped in\n", duplicatesFound ); break;
    case 2: printf( "WARNING: %d points with same coordinates found and averaged in\n", duplicatesFound );
    }
  }
  return 0;
}

// utility func used by compare::pcc
// no sanity check, we assume the model is clean
// and generated by sampling that allways generate content with color and normals
void convertModel( const mm::Model&         inputModel,
                   pcc_quality::commandPar& params,
                   PccPointCloud&           outputModel,
                   const bool               verbose ) {
  for ( size_t i = 0; i < inputModel.vertices.size() / 3; ++i ) {
    // push the positions
    outputModel.xyz.p.push_back( std::array<float, 3>(
      { inputModel.vertices[i * 3], inputModel.vertices[i * 3 + 1], inputModel.vertices[i * 3 + 2] } ) );
    // push the normals if any
    if ( inputModel.normals.size() > i * 3 + 2 )
      outputModel.normal.n.push_back( std::array<float, 3>(
        { inputModel.normals[i * 3], inputModel.normals[i * 3 + 1], inputModel.normals[i * 3 + 2] } ) );
    // push the colors if any
    if ( inputModel.colors.size() > i * 3 + 2 )
      outputModel.rgb.c.push_back(
        std::array<unsigned char, 3>( { (unsigned char)( std::roundf( inputModel.colors[i * 3] ) ),
                                        (unsigned char)( std::roundf( inputModel.colors[i * 3 + 1] ) ),
                                        (unsigned char)( std::roundf( inputModel.colors[i * 3 + 2] ) ) } ) );
  }
  outputModel.size = (long)outputModel.xyz.p.size();
  outputModel.bXyz = outputModel.size >= 1;
  if ( inputModel.colors.size() == inputModel.vertices.size() ) outputModel.bRgb = true;
  else outputModel.bRgb = false;
  if ( inputModel.normals.size() == inputModel.vertices.size() ) outputModel.bNormal = true;
  else outputModel.bNormal = false;
  outputModel.bLidar = false;

  outputModel.xyz.nbdup.resize( outputModel.xyz.p.size() );
  std::fill( outputModel.xyz.nbdup.begin(), outputModel.xyz.nbdup.end(), 1 );
  removeDuplicatePoints( outputModel, params.dropDuplicates, params.neighborsProc, verbose );
}

int Compare::pcc( const mm::Model&         modelA,
                  const mm::Model&         modelB,
                  const mm::Image&         mapA,
                  const mm::Image&         mapB,
                  pcc_quality::commandPar& params,
                  mm::Model&               outputA,
                  mm::Model&               outputB,
                  const bool               verbose ) {
  // 1 - sample the models if needed
  sampleIfNeeded( modelA, mapA, outputA );
  sampleIfNeeded( modelB, mapB, outputB );

  // 2 - transcode to PCC internal format
  pcc_processing::PccPointCloud inCloud1;
  pcc_processing::PccPointCloud inCloud2;

  convertModel( outputA, params, inCloud1, verbose );
  convertModel( outputB, params, inCloud2, verbose );

  // we use outputA as reference for signal dynamic if needed
  if ( params.resolution == 0 ) {
    glm::vec3 minBox, maxBox;
    mm::Geometry::computeBBox( outputA.vertices, minBox, maxBox );
    params.resolution = glm::length( maxBox - minBox );
  }
  //
  params.file1 = "dummy1.ply";  // could be any name !="", just force some pcc inner tests to pass
  params.file2 = "dummy2.ply";  // could be any name !="", just force some pcc inner tests to pass
  // compute plane metric if we have valid normal arrays
  params.c2c_only =
    !( outputA.normals.size() == outputA.vertices.size() && outputB.normals.size() == outputB.vertices.size() );
  // compute color metric if valid color arrays (no support for RGBA colors)
  params.bColor =
    params.bColor
    && ( outputA.colors.size() == outputA.vertices.size() && outputB.colors.size() == outputB.vertices.size() );
  params.mseSpace  = 1;
  params.nbThreads = 1;

  // 3 - compute the metric
  pcc_quality::qMetric qm;
  const double         similarPointThreshold = 1e-20;
  computeQualityMetric( inCloud1, inCloud1, inCloud2, params, qm, verbose, similarPointThreshold );

  // store results to compute statistics in finalize step
  _pccResults.push_back( std::make_pair( (uint32_t)_pccResults.size(), qm ) );

  //
  return 0;
}

// collect multi-frame statistics
void Compare::pccFinalize( void ) {
  if ( _pccResults.size() > 0 ) {
    Statistics::Results stats;

    Statistics::compute(
      _pccResults.size(), [&]( size_t i ) -> double { return _pccResults[i].second.c2c_psnr; }, stats );

    Statistics::printToLog( stats, "mseF, PSNR(p2point) ", std::cout );

    Statistics::compute(
      _pccResults.size(), [&]( size_t i ) -> double { return _pccResults[i].second.c2p_psnr; }, stats );

    Statistics::printToLog( stats, "mseF, PSNR(p2plane) ", std::cout );

    Statistics::compute(
      _pccResults.size(), [&]( size_t i ) -> double { return _pccResults[i].second.color_psnr[0]; }, stats );

    Statistics::printToLog( stats, "c[0],PSNRF          ", std::cout );

    Statistics::compute(
      _pccResults.size(), [&]( size_t i ) -> double { return _pccResults[i].second.color_psnr[1]; }, stats );

    Statistics::printToLog( stats, "c[1],PSNRF          ", std::cout );

    Statistics::compute(
      _pccResults.size(), [&]( size_t i ) -> double { return _pccResults[i].second.color_psnr[2]; }, stats );

    Statistics::printToLog( stats, "c[2],PSNRF          ", std::cout );
  }
}

// utility func used by compare::pcqm
// no sanity check, we assume the model is clean
// and generated by sampling that allways generate content with color and normals
void convertModel( const mm::Model& inputModel, PointSet& outputModel ) {
  // init bbox boundaries
  outputModel.xmin = outputModel.ymin = outputModel.zmin = std::numeric_limits<double>::max();
  outputModel.xmax = outputModel.ymax = outputModel.zmax = std::numeric_limits<double>::min();
  // note that RGBA is not supported - no error checking
  const bool haveColors = inputModel.colors.size() == inputModel.vertices.size();
  // copy data
  for ( size_t i = 0; i < inputModel.vertices.size() / 3; ++i ) {
    Point point;
    // push the positions
    point.x = inputModel.vertices[i * 3];
    point.y = inputModel.vertices[i * 3 + 1];
    point.z = inputModel.vertices[i * 3 + 2];
    // push the colors if any
    // PointSet ingests RGB8 stored on double.
    if ( haveColors ) {
      point.r = (double)inputModel.colors[i * 3];
      point.g = (double)inputModel.colors[i * 3 + 1];
      point.b = (double)inputModel.colors[i * 3 + 2];
    }
    // PCQM needs valid color attributes we generate a pure white
    else {
      point.r = point.g = point.b = (double)255;
    }
    // will add the point and update bbox
    outputModel.pts.push_back( point );
    outputModel.xmax = outputModel.xmax > point.x ? outputModel.xmax : point.x;
    outputModel.ymax = outputModel.ymax > point.y ? outputModel.ymax : point.y;
    outputModel.zmax = outputModel.zmax > point.z ? outputModel.zmax : point.z;
    outputModel.xmin = outputModel.xmin < point.x ? outputModel.xmin : point.x;
    outputModel.ymin = outputModel.ymin < point.y ? outputModel.ymin : point.y;
    outputModel.zmin = outputModel.zmin < point.z ? outputModel.zmin : point.z;
  }
}

int Compare::pcqm( const mm::Model& modelA,
                   const mm::Model& modelB,
                   const mm::Image& mapA,
                   const mm::Image& mapB,
                   const double     radiusCurvature,
                   const int        thresholdKnnSearch,
                   const double     radiusFactor,
                   mm::Model&       outputA,
                   mm::Model&       outputB,
                   const bool       verbose ) {
  // 1 - sample the models if needed
  sampleIfNeeded( modelA, mapA, outputA );
  sampleIfNeeded( modelB, mapB, outputB );

  // 2 - transcode to PCQM internal format
  PointSet inCloud1;
  PointSet inCloud2;

  convertModel( outputA, inCloud1 );
  convertModel( outputB, inCloud2 );

  // 3 - compute the metric
  // ModelA is Reference model
  // switch ref anf deg as in original PCQM (order matters)
  double pcqm =
    compute_pcqm( inCloud2, inCloud1, "reffile", "regfile", radiusCurvature, thresholdKnnSearch, radiusFactor );

  // compute PSNR
  // we use outputA as reference for PSNR signal dynamic
  glm::vec3 minBox, maxBox;
  mm::Geometry::computeBBox( outputA.vertices, minBox, maxBox );
  const double maxEnergy  = glm::length( maxBox - minBox );
  double       maxPcqm    = 1.0;
  double       pcqmScaled = ( pcqm / maxPcqm ) * maxEnergy;
  double       pcqmMse    = pcqmScaled * pcqmScaled;
  double       pcqmPsnr   = 10.0 * log10( maxEnergy * maxEnergy / pcqmMse );

  std::cout << "PCQM-PSNR=" << pcqmPsnr << std::endl;

  // store results to compute statistics
  _pcqmResults.push_back( std::make_tuple( (uint32_t)_pcqmResults.size(), pcqm, pcqmPsnr ) );

  //
  return 0;
}

//
void Compare::pcqmFinalize( void ) {
  if ( _pcqmResults.size() > 0 ) {
    Statistics::Results stats;

    Statistics::compute(
      _pcqmResults.size(), [&]( size_t i ) -> double { return std::get<1>( _pcqmResults[i] ); }, stats );

    Statistics::printToLog( stats, "PCQM ", std::cout );

    Statistics::compute(
      _pcqmResults.size(), [&]( size_t i ) -> double { return std::get<2>( _pcqmResults[i] ); }, stats );

    Statistics::printToLog( stats, "PCQM-PSNR ", std::cout );
  }
}

// utility function to generate sample on a sphere
// used by the ibsm method
void fibonacciSphere( std::vector<glm::vec3>& points,
                      uint32_t                samples   = 1,
                      const glm::vec3         rotParams = { 0.0F, 0.0F, 0.0F } ) {
  bool      rot = rotParams[2] ? true : false;
  glm::mat4 rotMatrix;

  if ( rot ) {
    float inclination = glm::radians( rotParams[0] );
    float azimuth     = glm::radians( rotParams[1] );
    float intrinsic   = glm::radians( rotParams[2] );
    float rotVec_r    = std::abs( std::sin( inclination ) );
    float rotVec_y    = std::cos( inclination );
    float rotVec_x    = std::cos( azimuth ) * rotVec_r;
    float rotVec_z    = std::sin( azimuth ) * rotVec_r;
    rotMatrix         = glm::rotate( intrinsic, glm::vec3( rotVec_x, rotVec_y, rotVec_z ) );
  }

  const double pi = std::atan( 1.0 ) * 4;

  // golden angle in radians
  float phi = (float)( pi * ( 3. - std::sqrt( 5. ) ) );

  for ( size_t i = 0; i < samples; ++i ) {
    float y      = 1 - ( i / float( samples - 1 ) ) * 2;  // y goes from 1 to - 1
    float radius = std::sqrt( 1 - y * y );                // radius at y
    float theta  = phi * i;                               // golden angle increment

    float x = std::cos( theta ) * radius;
    float z = std::sin( theta ) * radius;

    glm::vec3 pos = { x, y, z };
    if ( rot ) pos = glm::vec3( rotMatrix * glm::vec4( pos, 1.0 ) );

    points.push_back( pos );
  }
}

// compare two meshes using rasterization
int Compare::ibsm( const mm::Model&   modelA,
                   const mm::Model&   modelB,
                   const mm::Image&   mapA,
                   const mm::Image&   mapB,
                   const bool         disableReordering,
                   const uint32_t     resolution,
                   const uint32_t     cameraCount,
                   const glm::vec3&   camRotParams,
                   const std::string& renderer,
                   const std::string& outputPrefix,
                   const bool         disableCulling,
                   mm::Model&         outputA,
                   mm::Model&         outputB,
                   const bool         verbose ) {
  if ( renderer == "gl12_raster" && !_hwRendererInitialized ) {
    // now initialize OpenGL contexts if needed
    // this part is valid for all the frames
    if ( !_hwRenderer.initialize( resolution, resolution ) ) { return -1; }
    _hwRendererInitialized = true;
  }
  // place for the results
  IbsmResults res;
  size_t      maskSizeSum        = 0;  // store the sum for final computation of the mean
  size_t      unmatchedPixelsSum = 0;  // store the sum of unmatched pixels for reporting

  clock_t t1 = clock();
  if ( !disableReordering ) {
    // reorder the faces if needed, reordering is important for metric stability and
    // to get Infinite PSNR on equal meshes even with shuffled faces.
    mm::reorder( modelA, "oriented", outputA );
    mm::reorder( modelB, "oriented", outputB );
    if ( verbose )
      std::cout << "Time on mesh reordering = " << ( (float)( clock() - t1 ) ) / CLOCKS_PER_SEC << " sec." << std::endl;
  } else {
    // just replicate the input
    // outputs may have updated normals hereafter
    outputA = modelA;
    outputB = modelB;
    if ( verbose )
      std::cout << "Skipped reordering, time on mesh recopy = " << ( (float)( clock() - t1 ) ) / CLOCKS_PER_SEC
                << " sec." << std::endl;
  }

  const unsigned int width   = resolution;
  const unsigned int height  = resolution;
  glm::vec3          viewDir = { 0.0F, 0.0F, 1.0F };
  glm::vec3          viewUp  = { 0.0F, 1.0F, 0.0F };
  glm::vec3          bboxMin;
  glm::vec3          bboxMax;

  // allocate frame buffer - will be cleared by renderer
  std::vector<uint8_t> fbufferRef( width * height * 4 );
  std::vector<uint8_t> fbufferDis( width * height * 4 );
  // allocate depth buffer - will be cleared by renderer
  std::vector<float> zbufferRef( width * height );
  std::vector<float> zbufferDis( width * height );
  // computes the overall bbox
  glm::vec3 refBboxMin, refBboxMax;
  glm::vec3 disBboxMin, disBboxMax;
  mm::Geometry::computeBBox( outputA.vertices, refBboxMin, refBboxMax, true );
  mm::Geometry::computeBBox( outputB.vertices, disBboxMin, disBboxMax, true );
  mm::Geometry::computeBBox( refBboxMin, refBboxMax, disBboxMin, disBboxMax, bboxMin, bboxMax );
  double refDiagLength = glm::length( refBboxMax - refBboxMin );
  double disDiagLength = glm::length( disBboxMax - disBboxMin );
  res.boxRatio         = 100.0 * disDiagLength / refDiagLength;
  // default dynamic for Gl_raster, will be updated by sw_raster
  float sigDynamic = 1.0F;

  // prepare some camera directions
  std::vector<glm::vec3> camDir;
  fibonacciSphere( camDir, cameraCount, camRotParams );

  // for validation
  size_t depthNanCount = 0;
  size_t colorNanCount = 0;

  // now we render for each camera position
  for ( size_t camIdx = 0; camIdx < camDir.size(); ++camIdx ) {
    viewDir = camDir[camIdx];
    if ( glm::distance( glm::abs( viewDir ), glm::vec3( 0, 1, 0 ) ) < 1e-6 ) viewUp = glm::vec3( 0, 0, 1 );
    else viewUp = glm::vec3( 0, 1, 0 );

    if ( verbose ) {
      std::cout << "render viewDir= " << viewDir[0] << " " << viewDir[1] << " " << viewDir[2] << std::endl;
      std::cout << "render viewUp= " << viewUp[0] << " " << viewUp[1] << " " << viewUp[2] << std::endl;
    }
    clock_t t1 = clock();

    if ( renderer == "gl12_raster" ) {
      if ( disableCulling ) _hwRenderer.disableCulling();
      else _hwRenderer.enableCulling();

      _hwRenderer.render(
        &outputA, &mapA, fbufferRef, zbufferRef, width, height, viewDir, viewUp, bboxMin, bboxMax, true, verbose );
      _hwRenderer.render(
        &outputB, &mapB, fbufferDis, zbufferDis, width, height, viewDir, viewUp, bboxMin, bboxMax, true, verbose );
    } else {
      if ( disableCulling ) _swRenderer.disableCulling();
      else _swRenderer.enableCulling();

      _swRenderer.render(
        &outputA, &mapA, fbufferRef, zbufferRef, width, height, viewDir, viewUp, bboxMin, bboxMax, true, verbose );
      float depthRangeRef = _swRenderer.depthRange;

      _swRenderer.render(
        &outputB, &mapB, fbufferDis, zbufferDis, width, height, viewDir, viewUp, bboxMin, bboxMax, true, verbose );
      float depthRangeDis = _swRenderer.depthRange;

      if ( depthRangeRef != depthRangeDis ) {  // should never occur
        std::cout << "Warning: reference and distorted signal dynamics are different, " << depthRangeRef << " vs "
                  << depthRangeDis << std::endl;
      }
      sigDynamic = depthRangeDis;
      if ( verbose ) std::cout << "Signal Dynamic = " << depthRangeRef << std::endl;
    }

    clock_t t2 = clock();
    if ( verbose ) {
      std::cout << "Time on buffers rendering: " << ( (float)( t2 - t1 ) ) / CLOCKS_PER_SEC << " sec." << std::endl;
    }
    if ( outputPrefix != "" ) {
      const std::string fullPrefix =
        outputPrefix + "_" + std::to_string( _ibsmResults.size() ) + "_" + std::to_string( camIdx ) + "_";

      // Write image Y-flipped because OpenGL
      stbi_write_png( ( fullPrefix + "ref.png" ).c_str(),
                      width,
                      height,
                      4,
                      fbufferRef.data() + ( width * 4 * ( height - 1 ) ),
                      -(int)width * 4 );

      // Write image Y-flipped because OpenGL
      stbi_write_png( ( fullPrefix + "dis.png" ).c_str(),
                      width,
                      height,
                      4,
                      fbufferDis.data() + ( width * 4 * ( height - 1 ) ),
                      -(int)width * 4 );

      // converts depth to positive 8 bit for visualization
      std::vector<uint8_t> zbufferRef_8bits( zbufferRef.size(), 255 );
      std::vector<uint8_t> zbufferDis_8bits( zbufferRef.size(), 255 );

      for ( size_t i = 0; i < zbufferRef.size(); ++i ) {
        if ( fbufferRef[i * 4 + 3] != 0 ) {
          zbufferRef_8bits[i] = 255 - (uint8_t)( 255 * ( sigDynamic + zbufferRef[i] ) / sigDynamic );
        }
        if ( fbufferDis[i * 4 + 3] != 0 ) {
          zbufferDis_8bits[i] = 255 - (uint8_t)( 255 * ( sigDynamic + zbufferDis[i] ) / sigDynamic );
        }
      }

      // Write image Y-flipped because OpenGL
      stbi_write_png( ( fullPrefix + "ref_depth.png" ).c_str(),
                      width,
                      height,
                      1,
                      zbufferRef_8bits.data() + ( width * 1 * ( height - 1 ) ),
                      -(int)width * 1 );

      // Write image Y-flipped because OpenGL
      stbi_write_png( ( fullPrefix + "dis_depth.png" ).c_str(),
                      width,
                      height,
                      1,
                      zbufferDis_8bits.data() + ( width * 1 * ( height - 1 ) ),
                      -(int)width * 1 );
    }

    // 0 - compute the amount of pixels where there is a projection of Ref or Dist
    for ( size_t i = 0; i < fbufferRef.size() / 4; ++i ) {
      const uint8_t maskRef = fbufferRef[i * 4 + 3];
      const uint8_t maskDis = fbufferDis[i * 4 + 3];
      if ( maskRef != 0 && maskDis != 0 ) {
        maskSizeSum += 1;
      } else if ( maskRef != 0 || maskDis != 0 ) {
        unmatchedPixelsSum += 1;
      }
    }

    // A - now compute the Color Squared Error over the ref and dist images
    // store result in IbsmResults structures for convenience
    // but note that we store Squared Error into fields noted MSE
    for ( size_t i = 0; i < fbufferRef.size() / 4; ++i ) {
      const uint8_t maskRef = fbufferRef[i * 4 + 3];
      const uint8_t maskDis = fbufferDis[i * 4 + 3];
      // both object are projected on this pixel
      if ( maskRef != 0 && maskDis != 0 ) {
        // store YUV on vector of floats
        const glm::vec3 rgbRef( fbufferRef[i * 4 + 0], fbufferRef[i * 4 + 1], fbufferRef[i * 4 + 2] );
        const glm::vec3 rgbDis( fbufferDis[i * 4 + 0], fbufferDis[i * 4 + 1], fbufferDis[i * 4 + 2] );
        const glm::vec3 yuvRef = mm::rgbToYuvBt709_256( rgbRef );
        const glm::vec3 yuvDis = mm::rgbToYuvBt709_256( rgbDis );
        //
        for ( glm::vec3::length_type c = 0; c < 3; ++c ) {  // we skip the alpha channel
          // |I1 - I2|
          double pixel_cmp_sse_rgb = (double)rgbRef[c] - (double)rgbDis[c];
          double pixel_cmp_sse_yuv = (double)yuvRef[c] - (double)yuvDis[c];
          // |I1 - I2|^2
          pixel_cmp_sse_rgb = pixel_cmp_sse_rgb * pixel_cmp_sse_rgb;
          pixel_cmp_sse_yuv = pixel_cmp_sse_yuv * pixel_cmp_sse_yuv;
          // ensures color values are valid, otherwise skip the sample
          if ( std::isnan( pixel_cmp_sse_rgb ) || std::isnan( pixel_cmp_sse_yuv ) ) {
            pixel_cmp_sse_rgb = 0.0;
            pixel_cmp_sse_yuv = 0.0;
            colorNanCount++;
          }
          // Sum mean
          res.rgbMSE[c] = res.rgbMSE[c] + pixel_cmp_sse_rgb;
          res.yuvMSE[c] = res.yuvMSE[c] + pixel_cmp_sse_yuv;
        }
      }
      // else we skip ~ add 0, because no pixel exist in both buffers (faster processing)
    }

    // B - now compute the Geometric MSE over the ref and dist depth buffers
    // allways renormalize on an energy range of 255x255 to be coherent with rgb PSNR
    // store result in IbsmResults structures for convenience
    // but note that we store Squared Error into fields noted MSE

    for ( size_t i = 0; i < zbufferRef.size(); ++i ) {
      const uint8_t maskRef = fbufferRef[i * 4 + 3];
      const uint8_t maskDis = fbufferDis[i * 4 + 3];
      // both object are projected on this pixel
      if ( maskRef != 0 && maskDis != 0 ) {
        // |I1 - I2|
        double pixel_depth_sse = ( (double)zbufferRef[i] - (double)zbufferDis[i] ) * 255.0 / sigDynamic;
        // |I1 - I2|^2
        pixel_depth_sse = pixel_depth_sse * pixel_depth_sse;
        // ensures depth values are valid, otherwise skip the sample
        if ( std::isnan( pixel_depth_sse ) ) {
          pixel_depth_sse = 0.0;
          depthNanCount++;
        }
        // Sum mean
        res.depthMSE = res.depthMSE + pixel_depth_sse;
      }
      // else we skip ~ add 0, because no depth exist in both buffers (faster processing)
    }

    if ( verbose ) {
      clock_t t3 = clock();
      std::cout << "Time on MSE computing: " << ( (float)( t3 - t2 ) ) / CLOCKS_PER_SEC << " sec." << std::endl;
    }
  }

  // finally computes the MSE by dividing over total number of projected pixels
  for ( size_t c = 0; c < 3; ++c ) {
    res.rgbMSE[c] = res.rgbMSE[c] / (double)maskSizeSum;
    res.yuvMSE[c] = res.yuvMSE[c] / (double)maskSizeSum;
  }
  res.rgbMSE[3] = ( res.rgbMSE[0] + res.rgbMSE[1] + res.rgbMSE[2] ) / 3.0;
  res.yuvMSE[3] = ( res.yuvMSE[0] * 6.0 + res.yuvMSE[1] + res.yuvMSE[2] ) / 8.0;
  res.depthMSE  = res.depthMSE / (double)maskSizeSum;

  // compute the PSNRs (can be infinite)
  for ( size_t c = 0; c <= 3; ++c ) {
    res.rgbPSNR[c] = std::min( 999.99, 10.0 * log10( (double)( 255 * 255 ) / res.rgbMSE[c] ) );
    res.yuvPSNR[c] = std::min( 999.99, 10.0 * log10( (double)( 255 * 255 ) / res.yuvMSE[c] ) );
  }
  res.depthPSNR = std::min( 999.99, 10.0 * log10( (double)( 255 * 255 ) / res.depthMSE ) );

  // report the UnmatchedPixelPercentage - we may also report values over a given threshold to cout
  res.unmatchedPixelPercentage = ( maskSizeSum > 0 ) ? 100.0 * (double)unmatchedPixelsSum / (double)maskSizeSum : 100.0;

  if ( verbose ) {
    // Debug
    if ( depthNanCount != 0 ) {
      std::cout << "Warning: skipped " << depthNanCount << " NaN in depth buffer" << std::endl;
    }
    if ( colorNanCount != 0 ) {
      std::cout << "Warning: skipped " << colorNanCount << " NaN in color buffer" << std::endl;
    }

    if ( res.boxRatio < 99.5F || res.boxRatio > 100.5F ) {
      std::cout
        << "Warning: the size of the bounding box of reference and distorted models are quite different (see BoxRatio)."
        << "  IBSM results might not be accurate. Please perform a visual check of your models." << std::endl;
    }

    // output the results
    std::cout << "UnmatchedPixelPercentage = " << res.unmatchedPixelPercentage << std::endl;
    std::cout << "BoxRatio = " << res.boxRatio << std::endl;
    std::cout << "R   MSE  = " << res.rgbMSE[0] << std::endl;
    std::cout << "G   MSE  = " << res.rgbMSE[1] << std::endl;
    std::cout << "B   MSE  = " << res.rgbMSE[2] << std::endl;
    std::cout << "RGB MSE  = " << res.rgbMSE[3] << std::endl;
    std::cout << "Y   MSE  = " << res.yuvMSE[0] << std::endl;
    std::cout << "U   MSE  = " << res.yuvMSE[1] << std::endl;
    std::cout << "V   MSE  = " << res.yuvMSE[2] << std::endl;
    std::cout << "YUV MSE  = " << res.yuvMSE[3] << std::endl;
    std::cout << "GEO MSE  = " << res.depthMSE << std::endl;
    std::cout << "R   PSNR = " << res.rgbPSNR[0] << std::endl;
    std::cout << "G   PSNR = " << res.rgbPSNR[1] << std::endl;
    std::cout << "B   PSNR = " << res.rgbPSNR[2] << std::endl;
    std::cout << "RGB PSNR = " << res.rgbPSNR[3] << std::endl;
    std::cout << "Y   PSNR = " << res.yuvPSNR[0] << std::endl;
    std::cout << "U   PSNR = " << res.yuvPSNR[1] << std::endl;
    std::cout << "V   PSNR = " << res.yuvPSNR[2] << std::endl;
    std::cout << "YUV PSNR = " << res.yuvPSNR[3] << std::endl;
    std::cout << "GEO PSNR = " << res.depthPSNR << std::endl;
  }
  // store results to compute statistics
  _ibsmResults.push_back( std::make_pair( (uint32_t)_ibsmResults.size(), res ) );

  return 0;
}

void Compare::ibsmFinalize( void ) {
  if ( _ibsmResults.size() > 0 ) {
    Statistics::Results stats;

    Statistics::compute(
      _ibsmResults.size(), [&]( size_t i ) -> double { return _ibsmResults[i].second.boxRatio; }, stats );
    Statistics::printToLog( stats, "BoxRatio ", std::cout );

    Statistics::compute(
      _ibsmResults.size(),
      [&]( size_t i ) -> double { return _ibsmResults[i].second.unmatchedPixelPercentage; },
      stats );
    Statistics::printToLog( stats, "UnmatchedPixelPercentage ", std::cout );

    Statistics::compute(
      _ibsmResults.size(), [&]( size_t i ) -> double { return _ibsmResults[i].second.rgbPSNR[3]; }, stats );
    Statistics::printToLog( stats, "RGB PSNR ", std::cout );

    Statistics::compute(
      _ibsmResults.size(), [&]( size_t i ) -> double { return _ibsmResults[i].second.yuvPSNR[0]; }, stats );
    Statistics::printToLog( stats, "Y   PSNR ", std::cout );

    Statistics::compute(
      _ibsmResults.size(), [&]( size_t i ) -> double { return _ibsmResults[i].second.yuvPSNR[1]; }, stats );
    Statistics::printToLog( stats, "U   PSNR ", std::cout );

    Statistics::compute(
      _ibsmResults.size(), [&]( size_t i ) -> double { return _ibsmResults[i].second.yuvPSNR[2]; }, stats );
    Statistics::printToLog( stats, "V   PSNR ", std::cout );

    Statistics::compute(
      _ibsmResults.size(), [&]( size_t i ) -> double { return _ibsmResults[i].second.yuvPSNR[3]; }, stats );
    Statistics::printToLog( stats, "YUV PSNR ", std::cout );

    Statistics::compute(
      _ibsmResults.size(), [&]( size_t i ) -> double { return _ibsmResults[i].second.depthPSNR; }, stats );
    Statistics::printToLog( stats, "GEO PSNR ", std::cout );
  }
}

std::vector<double> Compare::getPccResults( const size_t index ) {
  std::vector<double> results;
  results.resize( 10, 0.0 );
  if ( index < _pccResults.size() ) {
    results[0] = ( std::min )( 999.99, _pccResults[index].second.c2c_psnr );
    results[1] = ( std::min )( 999.99, _pccResults[index].second.c2p_psnr );
    results[2] = ( std::min )( 999.99, _pccResults[index].second.color_psnr[0] );
    results[3] = ( std::min )( 999.99, _pccResults[index].second.color_psnr[1] );
    results[4] = ( std::min )( 999.99, _pccResults[index].second.color_psnr[2] );
    results[5] = ( std::min )( 999.99, _pccResults[index].second.c2c_hausdorff_psnr );
    results[6] = ( std::min )( 999.99, _pccResults[index].second.c2p_hausdorff_psnr );
    results[7] = ( std::min )( 999.99, _pccResults[index].second.color_rgb_hausdorff_psnr[0] );
    results[8] = ( std::min )( 999.99, _pccResults[index].second.color_rgb_hausdorff_psnr[1] );
    results[9] = ( std::min )( 999.99, _pccResults[index].second.color_rgb_hausdorff_psnr[2] );
  }
  return results;
}

std::vector<double> Compare::getFinalPccResults() {
  auto results = getPccResults( _pccResults.size() );
  if ( _pccResults.size() > 0 ) {
    for ( size_t i = 0; i < _pccResults.size(); i++ ) {
      const auto element = getPccResults( i );
      for ( size_t j = 0; j < results.size(); j++ ) results[j] += element[j];
    }
    for ( auto& v : results ) v /= (double)_pccResults.size();
  }
  return results;
}

std::vector<double> Compare::getPcqmResults( const size_t index ) {
  std::vector<double> results;
  results.resize( 2, 0.0 );
  if ( index < _pcqmResults.size() ) {
    results[0] = ( std::min )( 999.99, std::get<1>( _pcqmResults[index] ) );
    results[1] = ( std::min )( 999.99, std::get<2>( _pcqmResults[index] ) );
  }
  return results;
}

std::vector<double> Compare::getFinalPcqmResults() {
  auto results = getPcqmResults( _pcqmResults.size() );
  if ( _pcqmResults.size() > 0 ) {
    for ( size_t i = 0; i < _pcqmResults.size(); i++ ) {
      const auto element = getPcqmResults( i );
      for ( size_t j = 0; j < results.size(); j++ ) results[j] += element[j];
    }
    for ( auto& v : results ) v /= (double)_pcqmResults.size();
  }
  return results;
}

std::vector<double> Compare::getIbsmResults( const size_t index ) {
  std::vector<double> results;
  results.resize( 7, 0.0 );
  if ( index < _ibsmResults.size() ) {
    results[0] = ( std::min )( 999.99, _ibsmResults[index].second.boxRatio );
    results[1] = ( std::min )( 999.99, _ibsmResults[index].second.rgbPSNR[3] );
    results[2] = ( std::min )( 999.99, _ibsmResults[index].second.yuvPSNR[0] );
    results[3] = ( std::min )( 999.99, _ibsmResults[index].second.yuvPSNR[1] );
    results[4] = ( std::min )( 999.99, _ibsmResults[index].second.yuvPSNR[2] );
    results[5] = ( std::min )( 999.99, _ibsmResults[index].second.yuvPSNR[3] );
    results[6] = ( std::min )( 999.99, _ibsmResults[index].second.depthPSNR );
  }
  return results;
}

std::vector<double> Compare::getFinalIbsmResults() {
  auto results = getIbsmResults( _ibsmResults.size() );
  if ( _ibsmResults.size() > 0 ) {
    for ( size_t i = 0; i < _ibsmResults.size(); i++ ) {
      const auto element = getIbsmResults( i );
      for ( size_t j = 0; j < results.size(); j++ ) results[j] += element[j];
    }
    for ( auto& v : results ) v /= (double)_ibsmResults.size();
  }
  return results;
}
