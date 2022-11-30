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

#if USE_NATIVE_OSMESA
#define GLFW_EXPOSE_NATIVE_OSMESA
#include <GLFW/glfw3native.h>
#endif

// "implementation" done in mmIO
#include "stb_image_write.h"

// mathematics
#include <glm/vec3.hpp>
#include <glm/gtc/matrix_transform.hpp>

// internal headers
#include "mmIO.h"
#include "mmModel.h"
#include "mmImage.h"
#include "mmGeometry.h"
#include "mmRendererHw.h"

using namespace mm;

static const char* col_vertex_shader_text =
    "#version 110\n"
    "uniform mat4 MVP;\n"
    "uniform vec3 col;\n"
    "attribute vec3 vPos;\n"
    "varying vec3 color;\n"
    "void main()\n"
    "{\n"
    "    gl_Position = MVP * vec4(vPos, 1.0);\n"
    "    color = col;\n"
    "}\n";

static const char* col_fragment_shader_text =
    "#version 110\n"
    "varying vec3 color;\n"
    "void main()\n"
    "{\n"
    "    gl_FragColor = vec4(color, 1.0);\n"
    "}\n";

static const char* vertex_shader_text =
    "#version 110\n"
    "uniform mat4 MVP;\n"
    "attribute vec3 vCol;\n"
    "attribute vec3 vPos;\n"
    "varying vec3 color;\n"
    "void main()\n"
    "{\n"
    "    gl_Position = MVP * vec4(vPos, 1.0);\n"
    "    color = vCol / vec3(255.0);\n"
    "}\n";

static const char* fragment_shader_text =
    "#version 110\n"
    "varying vec3 color;\n"
    "void main()\n"
    "{\n"
    "    gl_FragColor = vec4(color, 1.0);\n"
    "}\n";

static const char* map_vertex_shader_text =
    "#version 110\n"
    "uniform mat4 MVP;\n"
    "attribute vec2 vTex;\n"
    "attribute vec3 vPos;\n"
    "varying vec2 texCoord;\n"
    "void main()\n"
    "{\n"
    "    gl_Position = MVP * vec4(vPos, 1.0);\n"
    "	 texCoord = vTex;\n"
    "}\n";

static const char* map_fragment_shader_text =
    "#version 110\n"
    "varying vec2 texCoord;\n"
    "uniform sampler2D texture;"
    "void main()\n"
    "{\n"
    "    //gl_FragColor = vec4(texCoord, 1.0, 1.0);\n"
    "    gl_FragColor = texture2D(texture, vec2(texCoord.x, 1.0 - texCoord.y));\n"
    "}\n";

// glfw error callback
static void error_callback( int error, const char* description ) {
  std::cerr << "Error: " << description << std::endl;
  ;
}

// gl error callback
void GLAPIENTRY MessageCallback( GLenum        source,
                                 GLenum        type,
                                 GLuint        id,
                                 GLenum        severity,
                                 GLsizei       length,
                                 const GLchar* message,
                                 const void*   userParam ) {
  fprintf( stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
           ( type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "" ), type, severity, message );
}

// open the output render context and associated hidden window
bool RendererHw::initialize( const unsigned int width, const unsigned int height ) {
  clock_t t1 = clock();

  _width  = width;
  _height = height;

  glfwSetErrorCallback( error_callback );

  glfwInitHint( GLFW_COCOA_MENUBAR, GLFW_FALSE );

  if ( !glfwInit() ) return false;

  glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 1 );
  glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 2 );
  glfwWindowHint( GLFW_VISIBLE, GLFW_FALSE );

  // size of the hidden window is not important,
  // we will use an FBO offscreen redering
  // so we use a small 64x64 size to preserve memory
  _window = glfwCreateWindow( 64, 64, "Hidden window", NULL, NULL );
  if ( !_window ) {
    std::cout << "Error: could not create window for given GL version" << std::endl;
    glfwTerminate();
    return false;
  }

  glfwMakeContextCurrent( _window );

  if ( !gladLoadGLLoader( (GLADloadproc)glfwGetProcAddress ) ) {
    std::cout << "Error: could not init glad context for given GL version" << std::endl;
    glfwTerminate();
    return false;
  }

  if ( !GLAD_GL_ARB_framebuffer_object ) {
    std::cout << "Error: ARB_framebuffer_object not supported" << std::endl;
    return false;
  }
  if ( !GLAD_GL_ARB_shader_objects ) {
    std::cout << "Error: GLAD_GL_ARB_shader_objects not supported" << std::endl;
    return false;
  }

  // During init, enable debug output
  glEnable( GL_DEBUG_OUTPUT );
  if ( GLAD_GL_ARB_debug_output ) { glDebugMessageCallbackARB( MessageCallback, 0 ); }

  std::cout << "prepare FBO for offscreen renders" << std::endl;
  //-------------------------
  glGenFramebuffers( 1, &_fbo );
  glBindFramebuffer( GL_FRAMEBUFFER, _fbo );
  //
  glGenRenderbuffers( 1, &_color_rb );
  glBindRenderbuffer( GL_RENDERBUFFER, _color_rb );
  glRenderbufferStorage( GL_RENDERBUFFER, GL_RGBA8, _width, _height );
  glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, _color_rb );
  //-------------------------
  glGenRenderbuffers( 1, &_depth_rb );
  glBindRenderbuffer( GL_RENDERBUFFER, _depth_rb );
  glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH_COMPONENT, _width, _height );
  glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _depth_rb );
  // Does the GPU support current FBO configuration?
  GLenum status;
  status = glCheckFramebufferStatus( GL_FRAMEBUFFER );
  switch ( status ) {
    case GL_FRAMEBUFFER_COMPLETE:
      std::cout << "Created FBO width=" << _width << " height=" << _height << std::endl;
      break;
    default:
      std::cout << "Error: could not create proper FBO" << std::endl;
      glfwTerminate();
      return false;
  }

  std::cout << "Time on GL setup: " << ( (float)( clock() - t1 ) ) / CLOCKS_PER_SEC << " sec." << std::endl;

  return true;
}

// release OpenGL resources
bool RendererHw::shutdown( void ) {
  // GL stuff
  glDeleteFramebuffers( 1, &_fbo );
  glDeleteRenderbuffers( 1, &_color_rb );
  glDeleteRenderbuffers( 1, &_depth_rb );

  // GLFW stuff
  glfwDestroyWindow( _window );
  glfwTerminate();

  return true;
}

bool RendererHw::render( Model*                model,
                         const Image*          map,
                         std::vector<uint8_t>& fbuffer,
                         std::vector<float>&   zbuffer,
                         const unsigned int    width,
                         const unsigned int    height,
                         const glm::vec3&      viewDir,
                         const glm::vec3&      viewUp,
                         const glm::vec3&      bboxMin,
                         const glm::vec3&      bboxMax,
                         bool                  useBBox,
                         const bool            verbose ) {
  const Model* rndModel = model;

  GLuint VAO, vertex_buffer, color_buffer, texcoord_buffer, triangles_buffer;
#ifdef __APPLE__
  GLhandleARB vertex_shader, fragment_shader, program;
#else
  GLuint vertex_shader, fragment_shader, program;
#endif
  GLint mvp_location, vpos_location, vcol_location, col_location, vtex_location, texture_location;

  clock_t t1 = clock();

  Model* tmpModel = NULL;

  // test if we render cpv and maps
  if ( model->trianglesuv.size() != 0 ) {
    if( verbose )
      std::cout << "Reindexing the model" << std::endl;
    t1       = clock();
    tmpModel = new Model();
    reindex( *model, *tmpModel );
    rndModel = tmpModel;
    if( verbose )
      std::cout << "Time on reindexing: " << ( (float)( clock() - t1 ) ) / CLOCKS_PER_SEC << " sec." << std::endl;
  }
  bool useCpv = rndModel->colors.size() == rndModel->vertices.size() && map == NULL;
  bool useMap = rndModel->uvcoords.size() != 0 && map != NULL;

  t1 = clock();

  // NOTE: OpenGL error checks have been omitted for brevity
  if( verbose )
    std::cout << "Create shader " << std::endl;

  vertex_shader   = glCreateShaderObjectARB( GL_VERTEX_SHADER_ARB );
  fragment_shader = glCreateShaderObjectARB( GL_FRAGMENT_SHADER_ARB );

  if ( useMap ) {    
    if( verbose )
      std::cout << "UseMap" << std::endl;
    glShaderSourceARB( vertex_shader, 1, &map_vertex_shader_text, NULL );
    glShaderSourceARB( fragment_shader, 1, &map_fragment_shader_text, NULL );
  } else if ( useCpv ) {
    if( verbose )
      std::cout << "UseCpv" << std::endl;
    glShaderSourceARB( vertex_shader, 1, &vertex_shader_text, NULL );
    glShaderSourceARB( fragment_shader, 1, &fragment_shader_text, NULL );
  } else {
    // use diffuse color shader
    if( verbose )
      std::cout << "No map no CPV, use diffuse color" << std::endl;
    glShaderSourceARB( vertex_shader, 1, &col_vertex_shader_text, NULL );
    glShaderSourceARB( fragment_shader, 1, &col_fragment_shader_text, NULL );
  }

  //
  GLint compiled = GL_FALSE;
  GLint linked   = GL_FALSE;
  char  infoLog[512];

  glCompileShaderARB( vertex_shader );

  glGetObjectParameterivARB( vertex_shader, GL_OBJECT_COMPILE_STATUS_ARB, &compiled );
  if ( !compiled ) {
    glGetInfoLogARB( vertex_shader, 512, NULL, infoLog );
    std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    return false;
  }
  //
  glCompileShaderARB( fragment_shader );
  //
  glGetObjectParameterivARB( fragment_shader, GL_OBJECT_COMPILE_STATUS_ARB, &compiled );
  if ( !compiled ) {
    glGetInfoLogARB( fragment_shader, 512, NULL, infoLog );
    std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    return false;
  }

  if( verbose )
    std::cout << "Assemble and link" << std::endl;
  program = glCreateProgramObjectARB();
  glAttachObjectARB( program, vertex_shader );
  glAttachObjectARB( program, fragment_shader );
  glDeleteObjectARB( vertex_shader );
  glDeleteObjectARB( fragment_shader );
  glLinkProgramARB( program );
  // check for linking errors
  glGetObjectParameterivARB( program, GL_OBJECT_LINK_STATUS_ARB, &linked );
  if ( !compiled ) {
    glGetInfoLogARB( program, 512, NULL, infoLog );
    std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    return false;
  }
  mvp_location  = glGetUniformLocationARB( program, "MVP" );
  vpos_location = glGetAttribLocationARB( program, "vPos" );
  if ( useMap ) {
    vtex_location    = glGetAttribLocationARB( program, "vTex" );
    texture_location = glGetUniformLocationARB( program, "texture" );
  } else if ( useCpv ) {
    vcol_location = glGetAttribLocationARB( program, "vCol" );
  } else {
    col_location = glGetUniformLocationARB( program, "col" );
  }

  if( verbose )
    std::cout << "Time on shader compilation: " << ( (float)( clock() - t1 ) ) / CLOCKS_PER_SEC << " sec." << std::endl;
  t1 = clock();  
  if( verbose )
    std::cout << "Upload model to GPU" << std::endl;

  glGenVertexArrays( 1, &VAO );
  glGenBuffersARB( 1, &vertex_buffer );     // VBO
  glGenBuffersARB( 1, &color_buffer );      // VBO
  glGenBuffersARB( 1, &texcoord_buffer );   // VBO
  glGenBuffersARB( 1, &triangles_buffer );  // EBO

  // bind the Vertex Array Object first,
  glBindVertexArray( VAO );

  // vertex coordinates
  glBindBufferARB( GL_ARRAY_BUFFER_ARB, vertex_buffer );
  glBufferDataARB( GL_ARRAY_BUFFER_ARB, rndModel->vertices.size() * sizeof( float ), rndModel->vertices.data(),
                   GL_STATIC_DRAW_ARB );

  glEnableVertexAttribArrayARB( vpos_location );
  glVertexAttribPointerARB( vpos_location, 3, GL_FLOAT, GL_FALSE, 3 * sizeof( float ), (void*)0 );

  // texture coordinates
  if ( useMap ) {
    glBindBufferARB( GL_ARRAY_BUFFER_ARB, texcoord_buffer );
    glBufferDataARB( GL_ARRAY_BUFFER_ARB, rndModel->uvcoords.size() * sizeof( float ), rndModel->uvcoords.data(),
                     GL_STATIC_DRAW_ARB );
    glEnableVertexAttribArrayARB( vtex_location );
    glVertexAttribPointerARB( vtex_location, 2, GL_FLOAT, GL_FALSE, 2 * sizeof( float ), (void*)0 );
  }

  // color per vertex
  if ( useCpv ) {
    glBindBufferARB( GL_ARRAY_BUFFER_ARB, color_buffer );
    glBufferDataARB( GL_ARRAY_BUFFER_ARB, rndModel->colors.size() * sizeof( float ), rndModel->colors.data(),
                     GL_STATIC_DRAW_ARB );
    glEnableVertexAttribArrayARB( vcol_location );
    glVertexAttribPointerARB( vcol_location, 3, GL_FLOAT, GL_FALSE, 3 * sizeof( float ), (void*)0 );
  }

  glBindBufferARB( GL_ELEMENT_ARRAY_BUFFER_ARB, triangles_buffer );
  glBufferDataARB( GL_ELEMENT_ARRAY_BUFFER_ARB, rndModel->triangles.size() * sizeof( unsigned int ),
                   rndModel->triangles.data(), GL_STATIC_DRAW_ARB );

  glEnableVertexAttribArrayARB( 0 );
  glBindBufferARB( GL_ARRAY_BUFFER_ARB, 0 );  // disable
  glBindVertexArray( 0 );                     // disable VAO

  if( verbose )
    std::cout << "Time on model upload to GPU: " << ( (float)( clock() - t1 ) ) / CLOCKS_PER_SEC << " sec." << std::endl;
  t1 = clock();

  // Handle texture map
  unsigned int texture;
  if ( useMap ) {
    t1 = clock();    
    if( verbose )
      std::cout << "Upload texture to GPU" << std::endl;
    glGenTextures( 1, &texture );
    glBindTexture( GL_TEXTURE_2D, texture );
    // set the texture wrapping parameters
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                     GL_CLAMP_TO_EDGE );  // set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    // set texture filtering parameters
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
    //
    GLuint inputFormat = GL_RGB;
    if ( map->nbc == 4 )
      inputFormat = GL_RGBA;
    else if ( map->nbc == 1 )
      inputFormat = GL_RED;
    // allways use RGB internal format
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, map->width, map->height, 0, inputFormat, GL_UNSIGNED_BYTE, map->data );
    // glGenerateMipmap(GL_TEXTURE_2D); // not sure if we shall use the mipmaps    
    if( verbose )
      std::cout << "Time on texture upload to GPU: " << ( (float)( clock() - t1 ) ) / CLOCKS_PER_SEC << " sec."
                << std::endl;
  } else {
    glBindTexture( GL_TEXTURE_2D, 0 );
  }

  // Init finished now we render  
  if( verbose )
    std::cout << "Start rendering" << std::endl;
  t1 = clock();

  // glfwGetFramebufferSize(window, &_width, &_height);
  float ratio = (float)width / (float)height;

  glBindFramebuffer( GL_FRAMEBUFFER, _fbo );

  glViewport( 0, 0, width, height );
  glClearColor( _clearColor.r, _clearColor.g, _clearColor.b, _clearColor.a );
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

  glEnable( GL_DEPTH_TEST );

  if ( _isCullingEnabled )
    glEnable( GL_CULL_FACE );
  else
    glDisable( GL_CULL_FACE );

  glm::mat4 mvp;  // model view projection
  glm::vec3 minPos, maxPos;

  // fit ortho viewpoint to model bbox
  if ( useBBox ) {
    minPos = bboxMin;
    maxPos = bboxMax;
  } else {
    Geometry::computeBBox( rndModel->vertices, minPos, maxPos );
  }
  glm::vec3 halfBox = ( maxPos - minPos ) * glm::vec3( 0.5 );
  glm::vec3 boxCtr  = minPos + halfBox;
  float     size    = glm::length( halfBox );
  size              = size + size / 100.0F;  // add 1% so the model does not touch the image borders
  glm::mat4 proj    = glm::ortho( -ratio * size, ratio * size, -size, size, 0.0F, 2.0F * size );
  glm::mat4 view    = glm::lookAt( boxCtr + viewDir * size, boxCtr, viewUp );

  if( verbose ) {
    std::cout << "BBox = " << minPos.x << "," << minPos.y << "," << minPos.z << "/" << maxPos.x << "," << maxPos.y << ","
              << maxPos.z << std::endl;
    std::cout << "Size = " << size << std::endl;
    std::cout << "Bctr = " << boxCtr.x << "," << boxCtr.y << "," << boxCtr.z << std::endl;
  }

  mvp = proj * view;

  // bind textures on corresponding texture units
  if ( useMap ) {
    glActiveTextureARB( GL_TEXTURE0_ARB );
    glBindTexture( GL_TEXTURE_2D, texture );
  }

  glUseProgramObjectARB( program );
  glBindVertexArray( VAO );
  if ( useMap ) {
    glUniform1iARB( texture_location, 0 );  // the texture sampler
  } else if ( !useCpv ) {
    glm::vec3 col = {1, 0, 0};
    glUniform3fvARB( col_location, 1, (const GLfloat*)( glm::value_ptr( col ) ) );
  }

  glUniformMatrix4fvARB( mvp_location, 1, GL_FALSE, (const GLfloat*)( glm::value_ptr( mvp ) ) );

  glDrawElements( GL_TRIANGLES,                        // mode
                  (GLuint)rndModel->triangles.size(),  // count (nb vertices)
                  GL_UNSIGNED_INT,                     // type
                  (void*)0                             // element array buffer offset
  );

  // Now we readback
  if( verbose ) 
    std::cout << "Time on render: " << ( (float)( clock() - t1 ) ) / CLOCKS_PER_SEC << " sec." << std::endl;
  t1 = clock();

  // frame buffer readback if needed
  if ( fbuffer.size() != 0 ) {    
    if( verbose ) 
      std::cout << "Color readback" << std::endl;
    glReadBuffer( GL_COLOR_ATTACHMENT0 );
    glReadPixels( 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, fbuffer.data() );
  }

  // depth buffer readback if needed
  if ( zbuffer.size() != 0 ) {
    if( verbose ) 
      std::cout << "Depth readback" << std::endl;
    // glReadPixels(0, 0, width, height, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, zbuffer.data());
    glReadPixels( 0, 0, width, height, GL_DEPTH_COMPONENT, GL_FLOAT, zbuffer.data() );
  }

  if( verbose ) 
    std::cout << "Time on readback: " << ( (float)( clock() - t1 ) ) / CLOCKS_PER_SEC << " sec." << std::endl;

  if ( tmpModel != NULL ) delete tmpModel;

  return true;
}

bool RendererHw::render( Model*             model,
                         const Image*       map,
                         const std::string& outputImage,
                         const std::string& outputDepth,
                         const unsigned int width,
                         const unsigned int height,
                         const glm::vec3&   viewDir,
                         const glm::vec3&   viewUp,
                         const glm::vec3&   bboxMin,
                         const glm::vec3&   bboxMax,
                         bool               useBBox,
                         const bool         verbose ) {
  // allocate depth buffer - will be init by RendererSw::clear
  std::vector<float> zbuffer( width * height );
  // allocate frame buffer - will be init by RendererSw::clear
  std::vector<uint8_t> fbuffer( width * height * 4 );

  // render the model into memory buffers
  render( model, map, fbuffer, zbuffer, width, height, viewDir, viewUp, bboxMin, bboxMax, useBBox, verbose );

  clock_t t1 = clock();

  if ( outputImage != "" ) {
    // Write image Y-flipped because OpenGL
    stbi_write_png( outputImage.c_str(), width, height, 4, fbuffer.data() + ( width * 4 * ( height - 1 ) ),
                    -(int)width * 4 );
  }

  if ( outputDepth != "" ) {
    // Write depth splitted on RGBA
    stbi_write_png( outputDepth.c_str(), width, height, 4, (char*)zbuffer.data() + ( width * 4 * ( height - 1 ) ),
                    -(int)width * 4 );
  }
  if( verbose )
    std::cout << "Time on saving: " << ( (float)( clock() - t1 ) ) / CLOCKS_PER_SEC << " sec." << std::endl;

  return true;
}
