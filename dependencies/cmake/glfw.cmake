cmake_minimum_required(VERSION 3.17)

include( ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/cmake/CPM.cmake )

set( DIR ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/glfw )
if( NOT EXISTS ${DIR} )
  CPMAddPackage( NAME              glfw
                GITHUB_REPOSITORY glfw/glfw
                GIT_TAG           3.3.2
                SOURCE_DIR        ${DIR}
                OPTIONS           "GLFW_BUILD_TESTS Off"
                                  "GLFW_BUILD_EXAMPLES Off"
                                  "GLFW_BUILD_DOCS Off"
                                  "GLFW_INSTALL Off"
                                  "GLFW_USE_HYBRID_HPG On" 
                DOWNLOAD_ONLY     YES  )
endif()
set( GLFW_BUILD_TESTS    off )
set( GLFW_BUILD_EXAMPLES off )
set( GLFW_BUILD_DOCS     off )
set( GLFW_INSTALL        off )
set( GLFW_USE_HYBRID_HPG off )
add_subdirectory(${DIR})
