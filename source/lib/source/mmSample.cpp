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
// Author: meshToPcFace method based on original face sampling code from Owlii
// *****************************************************************

#include <iostream>
#include <fstream>
#include <set>
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

using namespace mm;

// this algorithm was originally developped by Owlii
void Sample::meshToPcFace( const Model& input,
                           Model&       output,
                           const Image& tex_map,
                           size_t       resolution,
                           float        thickness,
                           bool         bilinear,
                           bool         logProgress ) {
  // computes the bounding box of the vertices
  glm::vec3 minPos, maxPos;
  Geometry::computeBBox( input.vertices, minPos, maxPos );
  std::cout << "minbox = " << minPos[0] << "," << minPos[1] << "," << minPos[2] << std::endl;
  std::cout << "maxbox = " << maxPos[0] << "," << maxPos[1] << "," << maxPos[2] << std::endl;

  // computes the sampling step
  glm::vec3 diag       = maxPos - minPos;
  float     boxMaxSize = std::max( diag.x, std::max( diag.y, diag.z ) );
  float     step       = boxMaxSize / resolution;
  std::cout << "step = " << step << std::endl;

  // to prevent storing duplicate points, we use a ModelBuilder
  ModelBuilder builder( output );

  size_t skipped = 0;  // number of degenerate triangles

  for ( size_t t = 0; t < input.triangles.size() / 3; t++ ) {
    if ( logProgress ) std::cout << '\r' << t << "/" << input.triangles.size() / 3 << std::flush;

    Vertex v1, v2, v3;

    fetchTriangle( input, t, input.uvcoords.size() != 0, input.colors.size() != 0, input.normals.size() != 0, v1, v2,
                   v3 );

    // check if triangle is not degenerate
    if ( Geometry::triangleArea( v1.pos, v2.pos, v3.pos ) < DBL_EPSILON ) {
      ++skipped;
      continue;
    }

    // compute face normal
    glm::vec3 normal;
    Geometry::triangleNormal( v1.pos, v2.pos, v3.pos, normal );

    // computes face dimensions for sampling
    glm::vec3 v12_norm = v2.pos - v1.pos;
    glm::vec3 v23_norm = v3.pos - v2.pos;
    float     l12      = glm::length( v12_norm );
    float     l23      = glm::length( v23_norm );
    for ( int i = 0; i < 3; i++ ) {
      v12_norm[i] = v12_norm[i] / l12;
      v23_norm[i] = v23_norm[i] / l23;
    }

    // do the sampling
    for ( float step12 = 0.f; step12 <= l12; step12 += step ) {
      for ( float step23 = 0.f; step23 <= step12 / l12 * l23; step23 += step ) {
        float step_normal_bdry = 0.f;
        while ( step_normal_bdry <= thickness ) { step_normal_bdry += step; }
        step_normal_bdry -= step;

        for ( float step_normal = -step_normal_bdry; step_normal <= step_normal_bdry; step_normal += step ) {
          Vertex v;
          v.pos       = v1.pos + step12 * v12_norm + step23 * v23_norm + step_normal * normal;
          v.nrm       = normal;
          v.hasNormal = true;

          // compute the color if any
          if ( input.uvcoords.size() != 0 && tex_map.data != NULL ) {  // use the texture map
            // compute UV
            const glm::vec2 uv{
                ( v1.uv[0] + step12 / l12 * ( v2.uv[0] - v1.uv[0] ) + step23 / l23 * ( v3.uv[0] - v2.uv[0] ) ),
                ( v1.uv[1] + step12 / l12 * ( v2.uv[1] - v1.uv[1] ) + step23 / l23 * ( v3.uv[1] - v2.uv[1] ) )};

            // fetch the color from the map
            if ( bilinear )
              texture2D_bilinear( tex_map, uv, v.col );
            else
              texture2D( tex_map, uv, v.col );
            v.hasColor = true;
          } else if ( input.colors.size() != 0 ) {  // use color per vertex
            v.col[0] = v1.col[0] + step12 / l12 * ( v2.col[0] - v1.col[0] ) + step23 / l23 * ( v3.col[0] - v2.col[0] );
            v.col[1] = v1.col[1] + step12 / l12 * ( v2.col[1] - v1.col[1] ) + step23 / l23 * ( v3.col[1] - v2.col[1] );
            v.col[2] = v1.col[2] + step12 / l12 * ( v2.col[2] - v1.col[2] ) + step23 / l23 * ( v3.col[2] - v2.col[2] );
            v.hasColor = true;
          }

          // add the vertex
          builder.pushVertex( v );
        }
      }
    }
  }
  if ( logProgress ) std::cout << std::endl;
  if ( skipped != 0 ) std::cout << "Skipped " << skipped << " degenerate triangles" << std::endl;
  if ( builder.foundCount != 0 ) std::cout << "Skipped " << builder.foundCount << " duplicate vertices" << std::endl;
  std::cout << "Generated " << output.vertices.size() / 3 << " points" << std::endl;
}

void Sample::meshToPcFace( const Model& input,
                           Model&       output,
                           const Image& tex_map,
                           size_t       nbSamplesMin,
                           size_t       nbSamplesMax,
                           size_t       maxIterations,
                           float        thickness,
                           bool         bilinear,
                           bool         logProgress,
                           size_t&      computedResolution ) {
  size_t resolution = 1024;
  meshToPcFace( input, output, tex_map, resolution, thickness, bilinear, logProgress );
  // search to init the algo bounds
  size_t minResolution = 0;
  size_t maxResolution = 0;
  //
  size_t iter = 0;
  while ( ( output.getPositionCount() < nbSamplesMin || output.getPositionCount() > nbSamplesMax ) &&
          iter < maxIterations ) {
    iter++;
    // let's refine
    if ( output.getPositionCount() < nbSamplesMin ) {  // need to add some points
      minResolution = resolution;
      if ( maxResolution == 0 ) {
        resolution = minResolution * 2;
      } else {
        resolution = minResolution + ( maxResolution - minResolution ) / 2;
      }
    }
    if ( output.getPositionCount() > nbSamplesMax ) {  // need to remove some points
      maxResolution = resolution;
      resolution    = minResolution + ( maxResolution - minResolution ) / 2;
    }
    std::cout << "  posCount=" << output.getPositionCount() << std::endl;
    std::cout << "  minResolution=" << minResolution << std::endl;
    std::cout << "  maxResolution=" << maxResolution << std::endl;
    std::cout << "  resolution=" << resolution << std::endl;
    //
    output.reset();
    meshToPcFace( input, output, tex_map, resolution, thickness, bilinear, logProgress );
  }
  computedResolution = resolution;
  std::cout << "algorithm ended after " << iter << " iterations " << std::endl;
}

// we use ray tracing to process the result, we could also use a rasterization (might be faster)
void Sample::meshToPcGrid( const Model& input,
                           Model&       output,
                           const Image& tex_map,
                           const size_t resolution,
                           const bool   bilinear,
                           const bool   logProgress,
                           bool         useNormal,
                           bool         useFixedPoint,
                           glm::vec3&   minPos,
                           glm::vec3&   maxPos,
                           const bool   verbose ) {
  // computes the bounding box of the vertices
  glm::vec3     minBox       = minPos;
  glm::vec3     maxBox       = maxPos;
  const int32_t fixedPoint16 = ( 1u << 16 );
  glm::vec3     stepSize;
  if ( minPos == maxPos ) {
    Geometry::computeBBox( input.vertices, minBox, maxBox );
    if( verbose ) {
      std::cout << "Computing positions range" << std::endl;    
      std::cout << "minBox = " << minBox[0] << "," << minBox[1] << "," << minBox[2] << std::endl;
      std::cout << "maxBox = " << maxBox[0] << "," << maxBox[1] << "," << maxBox[2] << std::endl;
      std::cout << "Transform bounding box to square box" << std::endl;
    }
    // hence sampling will be unform in the three dimensions
    Geometry::toCubicalBBox(
        minBox, maxBox );  // this will change the origin of the coordinate system (but it is just a translation)
    stepSize = ( maxBox - minBox ) * ( 1.0F / (float)( resolution - 1 ) );
  } else {
    if( verbose ) {
      std::cout << "Using parameter positions range" << std::endl;
      std::cout << "minPos = " << minPos[0] << "," << minPos[1] << "," << minPos[2] << std::endl;
      std::cout << "maxPos = " << maxPos[0] << "," << maxPos[1] << "," << maxPos[2] << std::endl;
    }
    if ( useFixedPoint ) {
      // converting the values to a fixed point representation
      // minBox(FP16) will be used in AAPS -> shift
      for ( int i = 0; i < 3; i++ ) {
        if ( minBox[i] > 0 )
          minBox[i] = ( std::floor( minBox[i] * fixedPoint16 ) ) / fixedPoint16;
        else
          minBox[i] = ( -1 ) * ( std::ceil( std::abs( minBox[i] ) * fixedPoint16 ) ) / fixedPoint16;
        if ( maxBox[i] > 0 ) {
          maxBox[i] = std::ceil( maxPos[i] * fixedPoint16 ) / fixedPoint16;
        } else
          maxBox[i] = ( -1 ) * ( std::floor( std::abs( maxBox[i] ) * fixedPoint16 ) ) / fixedPoint16;
      }
    }
    if( verbose ) {
      std::cout << "minBox = " << minBox[0] << "," << minBox[1] << "," << minBox[2] << std::endl;
      std::cout << "maxBox = " << maxBox[0] << "," << maxBox[1] << "," << maxBox[2] << std::endl;
    }
    const glm::vec3 diag  = maxBox - minBox;
    float           range = std::max( std::max( diag.x, diag.y ), diag.z );
    stepSize[0]           = range * ( 1.0F / (float)( resolution - 1 ) );
    stepSize[1]           = range * ( 1.0F / (float)( resolution - 1 ) );
    stepSize[2]           = range * ( 1.0F / (float)( resolution - 1 ) );
    if ( useFixedPoint ) {
      stepSize[0] = ( std::ceil( stepSize[0] * fixedPoint16 ) ) / fixedPoint16;
      stepSize[1] = ( std::ceil( stepSize[1] * fixedPoint16 ) ) / fixedPoint16;
      stepSize[2] = ( std::ceil( stepSize[2] * fixedPoint16 ) ) / fixedPoint16;
    }
  }

  if( verbose ) {
    std::cout << "stepSize = " << stepSize[0] << "," << stepSize[1] << "," << stepSize[2] << std::endl;
  }

  // we will now sample between min and max over the three dimensions, using resolution
  // by throwing rays from the three orthogonal faces of the box XY, XZ, YZ

  // to prevent storing duplicate points, we use a ModelBuilder
  ModelBuilder builder( output );

  size_t skipped = 0;  // number of degenerate triangles

  // for each triangle
  for ( size_t triIdx = 0; triIdx < input.triangles.size() / 3; ++triIdx ) {
    if ( logProgress ) std::cout << '\r' << triIdx << "/" << input.triangles.size() / 3 << std::flush;

    Vertex v1, v2, v3;

    fetchTriangle( input, triIdx, input.uvcoords.size() != 0, input.colors.size() != 0, input.normals.size() != 0, v1,
                   v2, v3 );

    // check if triangle is not degenerate
    if ( Geometry::triangleArea( v1.pos, v2.pos, v3.pos ) < DBL_EPSILON ) {
      ++skipped;
      continue;
    }

    // compute face normal
    glm::vec3 normal;
    Geometry::triangleNormal( v1.pos, v2.pos, v3.pos, normal );

    // extract the triangle bbox
    glm::vec3 triMinBox, triMaxBox;
    Geometry::triangleBBox( v1.pos, v2.pos, v3.pos, triMinBox, triMaxBox );

    // now find the Discrete range from global box to triangle box
    glm::vec3 lmin =
        glm::floor( ( triMinBox - minBox ) / stepSize );  // can lead to division by zero with flat box, handled later
    glm::vec3 lmax = glm::ceil( ( triMaxBox - minBox ) / stepSize );  // idem
    glm::vec3 lcnt = lmax - lmin;

    // now we will send rays on this triangle from the discreet steps of this box
    // rayTrace from the three main axis

    // reordering the search to start with the direction closest to the triangle normal
    glm::ivec3 mainAxisVector( 0, 1, 2 );
    // we want to preserve invariance with existing references if useNormal is disabled
    // so we do the following reordering only if option is enabled
    if ( useNormal ) {
      if ( ( std::abs( normal[0] ) >= std::abs( normal[1] ) ) && ( std::abs( normal[0] ) >= std::abs( normal[2] ) ) ) {
        if ( std::abs( normal[1] ) >= std::abs( normal[2] ) )
          mainAxisVector = glm::ivec3( 0, 1, 2 );
        else
          mainAxisVector = glm::ivec3( 0, 2, 1 );
      } else {
        if ( ( std::abs( normal[1] ) >= std::abs( normal[0] ) ) && ( std::abs( normal[1] ) >= std::abs( normal[2] ) ) ) {
          if ( std::abs( normal[0] ) >= std::abs( normal[2] ) )
            mainAxisVector = glm::ivec3( 1, 0, 2 );
          else
            mainAxisVector = glm::ivec3( 1, 2, 0 );
        } else {
          if ( std::abs( normal[0] ) >= std::abs( normal[1] ) )
            mainAxisVector = glm::ivec3( 2, 0, 1 );
          else
            mainAxisVector = glm::ivec3( 2, 1, 0 );
        }
      }
    }
    int mainAxisMaxIndex = useNormal ? 1 : 3;  // if useNormal is selected, we only need to check the first index

    for ( int mainAxisIndex = 0; mainAxisIndex < mainAxisMaxIndex; ++mainAxisIndex ) {
      glm::vec3::length_type mainAxis = mainAxisVector[mainAxisIndex];
      // axis swizzling
      glm::vec3::length_type secondAxis = 1;
      glm::vec3::length_type thirdAxis  = 2;
      if ( mainAxis == 1 ) {
        secondAxis = 0;
        thirdAxis  = 2;
      } else if ( mainAxis == 2 ) {
        secondAxis = 0;
        thirdAxis  = 1;
      }

      // skip this axis if box is null sized on one of the two other axis
      if ( minBox[secondAxis] == maxBox[secondAxis] || minBox[thirdAxis] == maxBox[thirdAxis] ) continue;

      // let's throw from mainAxis prependicular plane
      glm::vec3 rayOrigin    = {0.0, 0.0, 0.0};
      glm::vec3 rayDirection = {0.0, 0.0, 0.0};

      // on the main axis
      if ( stepSize[mainAxis] == 0.0F ) {  // handle stepSize[axis]==0
        // add small thress to be sure ray intersect in positive t
        rayOrigin[mainAxis] = minBox[mainAxis] - 0.5F;
      } else {
        rayOrigin[mainAxis] = minBox[mainAxis] + lmin[mainAxis] * stepSize[mainAxis];
      }
      // on main axis from min to max
      rayDirection[mainAxis] = 1.0;

      // iterate the second axis with i
      for ( size_t i = 0; i <= lcnt[secondAxis]; ++i ) {
        // iterate the third axis with j
        for ( size_t j = 0; j <= lcnt[thirdAxis]; ++j ) {
          // create the ray, starting from the face of the triangle bbox
          rayOrigin[secondAxis] = minBox[secondAxis] + ( lmin[secondAxis] + i ) * stepSize[secondAxis];
          rayOrigin[thirdAxis]  = minBox[thirdAxis] + ( lmin[thirdAxis] + j ) * stepSize[thirdAxis];

          //  triplet, x = t, y = u, z = v with t the parametric and (u,v) the barycentrics
          glm::vec3 res;

          // let' throw the ray toward the triangle
          if ( Geometry::evalRayTriangle( rayOrigin, rayDirection, v1.pos, v2.pos, v3.pos, res ) ) {
            // we convert the result into a point with color
            Vertex v;
            v.pos       = rayOrigin + rayDirection * res[0];
            v.nrm       = normal;
            v.hasNormal = true;

            // compute the color fi any
            // use the texture map
            if ( input.uvcoords.size() != 0 && tex_map.data != NULL ) {
              // use barycentric coordinates to extract point UV
              glm::vec2 uv = v1.uv * ( 1.0f - res.y - res.z ) + v2.uv * res.y + v3.uv * res.z;

              // fetch the color from the map
              if ( bilinear )
                texture2D_bilinear( tex_map, uv, v.col );
              else
                texture2D( tex_map, uv, v.col );

              v.hasColor = true;
              // v.col = v.col * rayDirection; --> for debugging, paints the color of the vertex according to the
              // direction
            }
            // use color per vertex
            else if ( input.colors.size() != 0 ) {
              // compute pixel color using barycentric coordinates
              v.col      = v1.col * ( 1.0f - res.y - res.z ) + v2.col * res.y + v3.col * res.z;
              v.hasColor = true;
            }

            // add the vertex
            builder.pushVertex( v );
          }
        }
      }
    }
  }
  if ( logProgress ) std::cout << std::endl;
  if ( verbose ) {
    if ( skipped != 0 ) std::cout << "Skipped " << skipped << " degenerate triangles" << std::endl;
    if ( builder.foundCount != 0 ) std::cout << "Skipped " << builder.foundCount << " duplicate vertices" << std::endl;
    std::cout << "Generated " << output.vertices.size() / 3 << " points" << std::endl;
  }
}

void Sample::meshToPcGrid( const Model& input,
                           Model&       output,
                           const Image& tex_map,
                           size_t       nbSamplesMin,
                           size_t       nbSamplesMax,
                           size_t       maxIterations,
                           bool         bilinear,
                           bool         logProgress,
                           bool         useNormal,
                           bool         useFixedPoint,
                           glm::vec3&   minPos,
                           glm::vec3&   maxPos,
                           size_t&      computedResolution ) {
  size_t resolution = 1024;
  meshToPcGrid( input, output, tex_map, resolution, bilinear, logProgress, useNormal, useFixedPoint, minPos, maxPos );
  // search to init the algo bounds
  size_t minResolution = 0;
  size_t maxResolution = 0;
  //
  size_t iter = 0;
  while ( ( output.getPositionCount() < nbSamplesMin || output.getPositionCount() > nbSamplesMax ) &&
          iter < maxIterations ) {
    iter++;
    // let's refine
    if ( output.getPositionCount() < nbSamplesMin ) {  // need to add some points
      minResolution = resolution;
      if ( maxResolution == 0 ) {
        resolution = minResolution * 2;
      } else {
        resolution = minResolution + ( maxResolution - minResolution ) / 2;
      }
    }
    if ( output.getPositionCount() > nbSamplesMax ) {  // need to remove some points
      maxResolution = resolution;
      resolution    = minResolution + ( maxResolution - minResolution ) / 2;
    }
    std::cout << "  posCount=" << output.getPositionCount() << std::endl;
    std::cout << "  minResolution=" << minResolution << std::endl;
    std::cout << "  maxResolution=" << maxResolution << std::endl;
    std::cout << "  resolution=" << resolution << std::endl;
    //
    output.reset();
    meshToPcGrid( input, output, tex_map, resolution, bilinear, logProgress, useNormal, useFixedPoint, minPos, maxPos );
  }
  computedResolution = resolution;
  std::cout << "algorithm ended after " << iter << " iterations " << std::endl;
}

// perform a reverse sampling of the texture map to generate mesh samples
// the color of the point is then using the texel color => no filtering
void Sample::meshToPcMap( const Model& input, Model& output, const Image& tex_map, bool logProgress ) {
  if ( input.uvcoords.size() == 0 ) {
    std::cerr << "Error: cannot back sample model, no UV coordinates" << std::endl;
    return;
  }
  if ( tex_map.width <= 0 || tex_map.height <= 0 || tex_map.nbc < 3 || tex_map.data == NULL ) {
    std::cerr << "Error: cannot back sample model, no valid texture map" << std::endl;
    return;
  }

  // to prevent storing duplicate points, we use a ModelBuilder
  ModelBuilder builder( output );

  size_t skipped = 0;  // number of degenerate triangles

  // For each triangle
  for ( size_t triIdx = 0; triIdx < input.triangles.size() / 3; ++triIdx ) {
    if ( logProgress ) std::cout << '\r' << triIdx << "/" << input.triangles.size() / 3 << std::flush;

    Vertex v1, v2, v3;

    fetchTriangle( input, triIdx, input.uvcoords.size() != 0, input.colors.size() != 0, input.normals.size() != 0, v1,
                   v2, v3 );

    // check if triangle is not degenerate
    if ( Geometry::triangleArea( v1.pos, v2.pos, v3.pos ) < DBL_EPSILON ) {
      ++skipped;
      continue;
    }

    // compute face normal
    glm::vec3 normal;
    Geometry::triangleNormal( v1.pos, v2.pos, v3.pos, normal );

    // compute the UVs bounding box
    glm::vec2 uvMin = {FLT_MAX, FLT_MAX};
    glm::vec2 uvMax = {-FLT_MAX, -FLT_MAX};
    uvMin           = glm::min( v3.uv, glm::min( v2.uv, glm::min( v1.uv, uvMin ) ) );
    uvMax           = glm::max( v3.uv, glm::max( v2.uv, glm::max( v1.uv, uvMax ) ) );

    // find the integer coordinates covered in the map
    glm::i32vec2 intUvMin = {( tex_map.width - 1 ) * uvMin.x, ( tex_map.height - 1 ) * uvMin.y};
    glm::i32vec2 intUvMax = {( tex_map.width - 1 ) * uvMax.x, ( tex_map.height - 1 ) * uvMax.y};

    // loop over the box in image space
    // if a pixel center is in the triangle then backproject
    // and create a new point with the pixel color
    for ( size_t i = intUvMin[0]; i <= intUvMax[0]; ++i ) {
      for ( size_t j = intUvMin[1]; j <= intUvMax[1]; ++j ) {
        // the new vertex
        Vertex v;
        // force to face normal
        v.hasNormal = true;
        v.nrm       = normal;
        // get the UV for the center of the pixel
        v.hasUVCoord = true;
        v.uv         = {( 0.5F + i ) / tex_map.width, ( 0.5F + j ) / tex_map.height};

        // test if this pixelUV is in the triangle UVs
        glm::vec3 bary;  // the barycentrics if success
        if ( Geometry::getBarycentric( v.uv, v1.uv, v2.uv, v3.uv, bary ) ) {
          // revert pixelUV to find point in 3D
          Geometry::triangleInterpolation( v1.pos, v2.pos, v3.pos, bary.x, bary.y, v.pos );

          // fetch the color
          texture2D( tex_map, v.uv, v.col );
          v.hasColor = true;

          // add to results
          builder.pushVertex( v );
        }
      }
    }
  }
  if ( logProgress ) std::cout << std::endl;
  if ( skipped != 0 ) std::cout << "Skipped " << skipped << " degenerate triangles" << std::endl;
  if ( builder.foundCount != 0 ) std::cout << "Skipped " << builder.foundCount << " duplicate vertices" << std::endl;
  std::cout << "Generated " << output.vertices.size() / 3 << " points" << std::endl;
}

// recursive body of meshtoPvDiv
void subdivideTriangle( const Vertex& v1,
                        const Vertex& v2,
                        const Vertex& v3,
                        const Image&  tex_map,
                        const float   thres,
                        const bool    mapThreshold,
                        const bool    bilinear,
                        ModelBuilder& output ) {
  // recursion stop criterion on area
  bool areaReached = Geometry::triangleArea( v1.pos, v2.pos, v3.pos ) < thres;

  // recursion stop criterion on texels adjacency
  if ( mapThreshold && tex_map.data != NULL ) {
    const glm::ivec2 mapSize = {tex_map.width, tex_map.height};
    glm::ivec2       mapCoord1, mapCoord2, mapCoord3;
    mapCoordClamped( v1.uv, mapSize, mapCoord1 );
    mapCoordClamped( v2.uv, mapSize, mapCoord2 );
    mapCoordClamped( v3.uv, mapSize, mapCoord3 );
    if ( std::abs( mapCoord1.x - mapCoord2.x ) <= 1 && std::abs( mapCoord1.x - mapCoord3.x ) <= 1 &&
         std::abs( mapCoord2.x - mapCoord3.x ) <= 1 && std::abs( mapCoord1.y - mapCoord2.y ) <= 1 &&
         std::abs( mapCoord1.y - mapCoord3.y ) <= 1 && std::abs( mapCoord2.y - mapCoord3.y ) <= 1 && areaReached ) {
      return;
    }
  } else if ( areaReached ) {
    return;
  }

  //
  glm::vec3 normal;
  Geometry::triangleNormal( v1.pos, v2.pos, v3.pos, normal );

  // the new vertices
  Vertex e1, e2, e3;
  // we sue v1 as reference in term of components to push
  e1.hasColor = e2.hasColor = e3.hasColor = v1.hasColor;
  e1.hasUVCoord = e2.hasUVCoord = e3.hasUVCoord = v1.hasUVCoord;
  // forces normals, we use generated per face ones - might be better as an option
  e1.hasNormal = e2.hasNormal = e3.hasNormal = true;

  // edge centers
  // (we do not interpolate normals but use face normal) - might be better as an option
  e1.pos = v1.pos * 0.5F + v2.pos * 0.5F;
  e1.col = v1.col * 0.5F + v2.col * 0.5F;
  e1.uv  = v1.uv * 0.5F + v2.uv * 0.5F;
  e1.nrm = normal;

  e2.pos = v2.pos * 0.5F + v3.pos * 0.5F;
  e2.col = v2.col * 0.5F + v3.col * 0.5F;
  e2.uv  = v2.uv * 0.5F + v3.uv * 0.5F;
  e2.nrm = normal;

  e3.pos = v3.pos * 0.5F + v1.pos * 0.5F;
  e3.col = v3.col * 0.5F + v1.col * 0.5F;
  e3.uv  = v3.uv * 0.5F + v1.uv * 0.5F;
  e3.nrm = normal;

  // push the new vertices
  output.pushVertex( e1, tex_map, bilinear );
  output.pushVertex( e2, tex_map, bilinear );
  output.pushVertex( e3, tex_map, bilinear );

  // go deeper in the subdivision
  subdivideTriangle( e1, e2, e3, tex_map, thres, mapThreshold, bilinear, output );
  subdivideTriangle( v1, e1, e3, tex_map, thres, mapThreshold, bilinear, output );
  subdivideTriangle( e1, v2, e2, tex_map, thres, mapThreshold, bilinear, output );
  subdivideTriangle( e2, v3, e3, tex_map, thres, mapThreshold, bilinear, output );
}

// Use triangle subdivision algorithm to perform the sampling
// Use simple subdiv scheme, stop criterion on triangle area or texture sample distance <= 1 pixel
void Sample::meshToPcDiv( const Model& input,
                          Model&       output,
                          const Image& tex_map,
                          float        areaThreshold,
                          bool         mapThreshold,
                          bool         bilinear,
                          bool         logProgress ) {
  // number of degenerate triangles
  size_t skipped = 0;

  // to prevent storing duplicate points, we use a ModelBuilder
  ModelBuilder builder( output );

  // For each triangle
  for ( size_t triIdx = 0; triIdx < input.triangles.size() / 3; ++triIdx ) {
    if ( logProgress ) std::cout << '\r' << triIdx << "/" << input.triangles.size() / 3 << std::flush;

    Vertex v1, v2, v3;

    fetchTriangle( input, triIdx, input.uvcoords.size() != 0, input.colors.size() != 0, input.normals.size() != 0, v1,
                   v2, v3 );

    // check if triangle is not degenerate
    if ( Geometry::triangleArea( v1.pos, v2.pos, v3.pos ) < DBL_EPSILON ) {
      ++skipped;
      continue;
    }

    // compute face normal (forces) - might be better as an option
    glm::vec3 normal;
    Geometry::triangleNormal( v1.pos, v2.pos, v3.pos, normal );
    v1.nrm = v2.nrm = v3.nrm = normal;
    v1.hasNormal = v2.hasNormal = v3.hasNormal = true;

    // push the vertices
    builder.pushVertex( v1, tex_map, bilinear );
    builder.pushVertex( v2, tex_map, bilinear );
    builder.pushVertex( v3, tex_map, bilinear );

    // subdivide recursively
    subdivideTriangle( v1, v2, v3, tex_map, areaThreshold, mapThreshold, bilinear, builder );
  }
  if ( logProgress ) std::cout << std::endl;
  if ( skipped != 0 ) std::cout << "Skipped " << skipped << " degenerate triangles" << std::endl;
  if ( builder.foundCount != 0 ) std::cout << "Handled " << builder.foundCount << " duplicate vertices" << std::endl;
  std::cout << "Generated " << output.vertices.size() / 3 << " points" << std::endl;
}

void Sample::meshToPcDiv( const Model& input,
                          Model&       output,
                          const Image& tex_map,
                          size_t       nbSamplesMin,
                          size_t       nbSamplesMax,
                          size_t       maxIterations,
                          bool         bilinear,
                          bool         logProgress,
                          float&       computedThres ) {
  float value = 1.0;
  meshToPcDiv( input, output, tex_map, value, 0, bilinear, logProgress );
  // search to init the algo bounds
  float minBound = 0;
  float maxBound = 0;
  //
  size_t iter = 0;
  while ( ( output.getPositionCount() < nbSamplesMin || output.getPositionCount() > nbSamplesMax ) &&
          iter < maxIterations ) {
    iter++;
    // let's refine
    if ( output.getPositionCount() > nbSamplesMin ) {  // need to remove some points
      minBound = value;
      if ( maxBound == 0 ) {
        value = minBound * 2;
      } else {
        value = minBound + ( maxBound - minBound ) / 2;
      }
    }
    if ( output.getPositionCount() < nbSamplesMax ) {  // need to add some points
      maxBound = value;
      value    = minBound + ( maxBound - minBound ) / 2;
    }
    std::cout << "  posCount=" << output.getPositionCount() << std::endl;
    std::cout << "  minBound=" << minBound << std::endl;
    std::cout << "  maxBound=" << maxBound << std::endl;
    std::cout << "  value=" << value << std::endl;
    //
    output.reset();
    meshToPcDiv( input, output, tex_map, value, 0, bilinear, logProgress );
  }

  computedThres = value;

  std::cout << "algorithm ended after " << iter << " iterations " << std::endl;
}

//                      // 
//          v2          // 
//   		    /\          // 
//         /  \         // 
//     e1 /----\ e2     // 
//       / \  / \       // 
//      /   \/   \      // 
//    v1 -------- v3    // 
//          e3          // 
//                      // 
void subdivideTriangleEdge( const Vertex& v1,
                            const Vertex& v2,
                            const Vertex& v3,
                            const Image&  tex_map,
                            const float   lengthThreshold,
                            const bool    bilinear,
                            ModelBuilder& output ) {
  // the face normal
  glm::vec3 normal;
  Geometry::triangleNormal( v1.pos, v2.pos, v3.pos, normal );

  // do we split the edges: length(edge)/2 >= threshold
  const bool split1 = glm::length( v2.pos - v1.pos ) * 0.5F >= lengthThreshold;
  const bool split2 = glm::length( v2.pos - v3.pos ) * 0.5F >= lengthThreshold;
  const bool split3 = glm::length( v3.pos - v1.pos ) * 0.5F >= lengthThreshold;

  // early return if threshold reached for each edge
  if ( !split1 && !split2 && !split3 ) return;

  // the edge centers ~ the potential new vertices
  Vertex e1, e2, e3;
  // we use v1 as reference in term of components to push
  e1.hasColor = e2.hasColor = e3.hasColor = v1.hasColor;
  e1.hasUVCoord = e2.hasUVCoord = e3.hasUVCoord = v1.hasUVCoord;
  // forces normals, we use generated per face ones
  e1.hasNormal = e2.hasNormal = e3.hasNormal = true;

  // compute and push the new edge centers if needed,
  if ( split1 ) {
    e1.pos = ( v1.pos + v2.pos ) * 0.5F;
    e1.col = ( v1.col + v2.col ) * 0.5F;
    e1.uv  = ( v1.uv + v2.uv ) * 0.5F;
    e1.nrm = normal;
    output.pushVertex( e1, tex_map, bilinear );
  }
  if ( split2 ) {
    e2.pos = ( v2.pos + v3.pos ) * 0.5F;
    e2.col = ( v2.col + v3.col ) * 0.5F;
    e2.uv  = ( v2.uv + v3.uv ) * 0.5F;
    e2.nrm = normal;
    output.pushVertex( e2, tex_map, bilinear );
  }
  if ( split3 ) {
    e3.pos = ( v3.pos + v1.pos ) * 0.5F;
    e3.col = ( v3.col + v1.col ) * 0.5F;
    e3.uv  = ( v3.uv + v1.uv ) * 0.5F;
    e3.nrm = normal;
    output.pushVertex( e3, tex_map, bilinear );
  }

  // go deeper in the subdivision if needed
  // three edge split
  if ( split1 && split2 && split3 ) {
    subdivideTriangleEdge( e1, e2, e3, tex_map, lengthThreshold, bilinear, output );
    subdivideTriangleEdge( v1, e1, e3, tex_map, lengthThreshold, bilinear, output );
    subdivideTriangleEdge( e1, v2, e2, tex_map, lengthThreshold, bilinear, output );
    subdivideTriangleEdge( e2, v3, e3, tex_map, lengthThreshold, bilinear, output );
    return;
  }
  // two edge split
  if ( !split1 && split2 && split3 ) {
    subdivideTriangleEdge( v1, v2, e3, tex_map, lengthThreshold, bilinear, output );
    subdivideTriangleEdge( e3, v2, e2, tex_map, lengthThreshold, bilinear, output );
    subdivideTriangleEdge( e2, v3, e3, tex_map, lengthThreshold, bilinear, output );
    return;
  }
  if ( split1 && !split2 && split3 ) {
    subdivideTriangleEdge( v1, e1, e3, tex_map, lengthThreshold, bilinear, output );
    subdivideTriangleEdge( e1, v2, e3, tex_map, lengthThreshold, bilinear, output );
    subdivideTriangleEdge( v2, v3, e3, tex_map, lengthThreshold, bilinear, output );
    return;
  }
  if ( split1 && split2 && !split3 ) {
    subdivideTriangleEdge( v1, e1, v3, tex_map, lengthThreshold, bilinear, output );
    subdivideTriangleEdge( e1, e2, v3, tex_map, lengthThreshold, bilinear, output );
    subdivideTriangleEdge( e1, v2, e2, tex_map, lengthThreshold, bilinear, output );
    return;
  }
  // one edge split
  if ( !split1 && !split2 && split3 ) {
    subdivideTriangleEdge( v1, v2, e3, tex_map, lengthThreshold, bilinear, output );
    subdivideTriangleEdge( v2, v3, e3, tex_map, lengthThreshold, bilinear, output );
    return;
  }
  if ( !split1 && split2 && !split3 ) {
    subdivideTriangleEdge( v1, v2, e2, tex_map, lengthThreshold, bilinear, output );
    subdivideTriangleEdge( v1, e2, v3, tex_map, lengthThreshold, bilinear, output );
    return;
  }
  if ( split1 && !split2 && !split3 ) {
    subdivideTriangleEdge( v1, e1, v3, tex_map, lengthThreshold, bilinear, output );
    subdivideTriangleEdge( e1, v2, v3, tex_map, lengthThreshold, bilinear, output );
    return;
  }
}

// Use triangle subdivision algorithm to perform the sampling
// Use subdiv scheme without T-vertices, stop criterion on edge size
void Sample::meshToPcDivEdge( const Model& input,
                              Model&       output,
                              const Image& tex_map,
                              float        lengthThreshold,
                              size_t       resolution,
                              bool         bilinear,
                              bool         logProgress,
                              float&       computedThres ) {
  float length = lengthThreshold;

  if ( length == 0 ) {
    std::cout << "Overriding lengthThreshold from resolution=" << resolution << std::endl;
    if ( resolution == 0 ) {
      std::cout << "Error: resolution must be > 0 if lengthThreshold = 0.0" << std::endl;
      return;
    }
    // computes the bounding box of the vertices
    glm::vec3 minPos, maxPos;
    Geometry::computeBBox( input.vertices, minPos, maxPos );
    std::cout << "  minbox = " << minPos[0] << "," << minPos[1] << "," << minPos[2] << std::endl;
    std::cout << "  maxbox = " << maxPos[0] << "," << maxPos[1] << "," << maxPos[2] << std::endl;
    // computes the edge length threshold from resolution of the largest size of the bbox
    const glm::vec3 diag       = maxPos - minPos;
    const float     boxMaxSize = std::max( diag.x, std::max( diag.y, diag.z ) );
    length                     = boxMaxSize / resolution;
    std::cout << "  lengthThreshold = " << length << std::endl;
    computedThres = length;  // forwards to caller, only if internally computed
  }

  // to prevent storing duplicate points, we use a ModelBuilder
  ModelBuilder builder( output );

  // number of degenerate triangles
  size_t skipped = 0;

  // For each triangle
  for ( size_t triIdx = 0; triIdx < input.triangles.size() / 3; ++triIdx ) {
    if ( logProgress ) std::cout << '\r' << triIdx << "/" << input.triangles.size() / 3 << std::flush;

    Vertex v1, v2, v3;

    fetchTriangle( input, triIdx, input.uvcoords.size() != 0, input.colors.size() != 0, input.normals.size() != 0, v1,
                   v2, v3 );

    // check if triangle is not degenerate
    if ( Geometry::triangleArea( v1.pos, v2.pos, v3.pos ) < DBL_EPSILON ) {
      ++skipped;
      continue;
    }

    // compute face normal (forces) - might be better as an option
    glm::vec3 normal;
    Geometry::triangleNormal( v1.pos, v2.pos, v3.pos, normal );
    v1.nrm = v2.nrm = v3.nrm = normal;
    v1.hasNormal = v2.hasNormal = v3.hasNormal = true;

    // push the vertices if needed
    builder.pushVertex( v1, tex_map, bilinear );
    builder.pushVertex( v2, tex_map, bilinear );
    builder.pushVertex( v3, tex_map, bilinear );

    // subdivide recursively
    subdivideTriangleEdge( v1, v2, v3, tex_map, length, bilinear, builder );
  }
  if ( logProgress ) std::cout << std::endl;
  if ( skipped != 0 ) std::cout << "Skipped " << skipped << " degenerate triangles" << std::endl;
  if ( builder.foundCount != 0 ) std::cout << "Handled " << builder.foundCount << " duplicate vertices" << std::endl;
  std::cout << "Generated " << output.vertices.size() / 3 << " points" << std::endl;
}

void Sample::meshToPcDivEdge( const Model& input,
                              Model&       output,
                              const Image& tex_map,
                              size_t       nbSamplesMin,
                              size_t       nbSamplesMax,
                              size_t       maxIterations,
                              bool         bilinear,
                              bool         logProgress,
                              float&       computedThres ) {
  float value = 1.0;
  float unused;
  meshToPcDivEdge( input, output, tex_map, value, 0, bilinear, logProgress, unused );
  // search to init the algo bounds
  float minBound = 0;
  float maxBound = 0;
  //
  size_t iter = 0;
  while ( ( output.getPositionCount() < nbSamplesMin || output.getPositionCount() > nbSamplesMax ) &&
          iter < maxIterations ) {
    iter++;
    // let's refine
    if ( output.getPositionCount() > nbSamplesMin ) {  // need to remove some points
      minBound = value;
      if ( maxBound == 0 ) {
        value = minBound * 2;
      } else {
        value = minBound + ( maxBound - minBound ) / 2;
      }
    }
    if ( output.getPositionCount() < nbSamplesMax ) {  // need to add some points
      maxBound = value;
      value    = minBound + ( maxBound - minBound ) / 2;
    }
    std::cout << "  posCount=" << output.getPositionCount() << std::endl;
    std::cout << "  minBound=" << minBound << std::endl;
    std::cout << "  maxBound=" << maxBound << std::endl;
    std::cout << "  value=" << value << std::endl;
    //
    output.reset();
    meshToPcDivEdge( input, output, tex_map, value, 0, bilinear, logProgress, unused );
  }
  computedThres = value;
  std::cout << "algorithm ended after " << iter << " iterations " << std::endl;
}

// Use uniform sampling algorithm
// Stop criterion generated point count
// Implementation of the algorithm described in
// http://extremelearning.com.au/evenly-distributing-points-in-a-triangle/

void Sample::meshToPcPrnd( const Model& input,
                           Model&       output,
                           const Image& tex_map,
                           size_t       targetPointCount,
                           bool         bilinear,
                           bool         logProgress ) {
  // number of degenerate triangles
  size_t skipped = 0;

  // to prevent storing duplicate points, we use a ModelBuilder
  ModelBuilder builder( output );
  double       totalArea = 0.0f;
  for ( size_t triIdx = 0; triIdx < input.triangles.size() / 3; ++triIdx ) {
    Vertex v1, v2, v3;

    fetchTriangle( input, triIdx, input.uvcoords.size() != 0, input.colors.size() != 0, input.normals.size() != 0, v1,
                   v2, v3 );
    totalArea += Geometry::triangleArea( v1.pos, v2.pos, v3.pos );
  }

  const auto g  = 1.0f / 1.32471795572f;
  const auto g2 = g * g;

  // For each triangle
  for ( size_t triIdx = 0; triIdx < input.triangles.size() / 3; ++triIdx ) {
    if ( logProgress ) std::cout << '\r' << triIdx << "/" << input.triangles.size() / 3 << std::flush;

    Vertex v1, v2, v3;
    fetchTriangle( input, triIdx, input.uvcoords.size() != 0, input.colors.size() != 0, input.normals.size() != 0, v1,
                   v2, v3 );

    // check if triangle is not degenerate
    if ( Geometry::triangleArea( v1.pos, v2.pos, v3.pos ) < DBL_EPSILON ) {
      ++skipped;
      continue;
    }

    const auto triArea    = Geometry::triangleArea( v1.pos, v2.pos, v3.pos );
    const auto pointCount = std::ceil( targetPointCount * triArea / totalArea );

    // compute face normal (forces) - might be better as an option
    glm::vec3 normal;
    Geometry::triangleNormal( v1.pos, v2.pos, v3.pos, normal );
    v1.nrm = v2.nrm = v3.nrm = normal;
    v1.hasNormal = v2.hasNormal = v3.hasNormal = true;

    // push the vertices
    builder.pushVertex( v1, tex_map, bilinear );
    builder.pushVertex( v2, tex_map, bilinear );
    builder.pushVertex( v3, tex_map, bilinear );

    const auto d12 = glm::distance( v1.pos, v2.pos );
    const auto d23 = glm::distance( v2.pos, v3.pos );
    const auto d31 = glm::distance( v3.pos, v1.pos );

    glm::vec3 dpos0, dpos1, pos;
    glm::vec2 duv0, duv1, uv;
    if ( d12 >= d23 && d12 >= d31 ) {
      uv   = v3.uv;
      duv0 = v1.uv - v3.uv;
      duv1 = v2.uv - v3.uv;

      pos   = v3.pos;
      dpos0 = v1.pos - v3.pos;
      dpos1 = v2.pos - v3.pos;
    } else if ( d31 >= d23 ) {
      uv   = v2.uv;
      duv0 = v1.uv - v2.uv;
      duv1 = v3.uv - v2.uv;

      pos   = v2.pos;
      dpos0 = v1.pos - v2.pos;
      dpos1 = v3.pos - v2.pos;
    } else {
      uv   = v1.uv;
      duv0 = v2.uv - v1.uv;
      duv1 = v3.uv - v1.uv;

      pos   = v1.pos;
      dpos0 = v2.pos - v1.pos;
      dpos1 = v3.pos - v1.pos;
    }

    // the new vertices
    Vertex vertex;
    // we use v1 as reference in term of components to push
    vertex.hasColor   = v1.hasColor;
    vertex.hasUVCoord = v1.hasUVCoord;
    // forces normals, we use generated per face ones
    vertex.hasNormal = true;

    for ( int i = 1; i < pointCount; ++i ) {
      const auto r1 = i * g;
      const auto r2 = i * g2;
      auto       x  = r1 - std::floor( r1 );
      auto       y  = r2 - std::floor( r2 );
      if ( x + y > 1.0f ) {
        x = 1.0f - x;
        y = 1.0f - y;
      }
      vertex.pos = pos + x * dpos0 + y * dpos1;
      vertex.uv  = uv + x * duv0 + y * duv1;
      vertex.nrm = normal;
      builder.pushVertex( vertex, tex_map, bilinear );
    }
  }
  if ( logProgress ) std::cout << std::endl;
  if ( skipped != 0 ) std::cout << "Skipped " << skipped << " degenerate triangles" << std::endl;
  if ( builder.foundCount != 0 ) std::cout << "Handled " << builder.foundCount << " duplicate vertices" << std::endl;
  std::cout << "Generated " << output.vertices.size() / 3 << " points" << std::endl;
}
