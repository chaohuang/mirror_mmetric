cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

PROJECT( mmlib )

###################################
# input sources and headers 
###################################

file(GLOB MM_LIB_INC ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)
file(GLOB MM_LIB_SRC ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp)

set(GLAD_SRC ${CMAKE_CURRENT_SOURCE_DIR}/../../dependencies/glad/src/glad.c )
set(PCQM_SRC ${CMAKE_CURRENT_SOURCE_DIR}/../../dependencies/pcqm/pcqm.cpp )
set(PCQM_INC ${CMAKE_CURRENT_SOURCE_DIR}/../../dependencies/pcqm/pcqm.h 
             ${CMAKE_CURRENT_SOURCE_DIR}/../../dependencies/pcqm/PointSet.h
             ${CMAKE_CURRENT_SOURCE_DIR}/../../dependencies/pcqm/resources.h
             ${CMAKE_CURRENT_SOURCE_DIR}/../../dependencies/pcqm/utilities.h )
set(PCC_SRC  ${CMAKE_CURRENT_SOURCE_DIR}/../../dependencies/dmetric/source/pcc_distortion.cpp 
             ${CMAKE_CURRENT_SOURCE_DIR}/../../dependencies/dmetric/source/pcc_processing.cpp )
set(PCC_INC  ${CMAKE_CURRENT_SOURCE_DIR}/../../dependencies/dmetric/source/pcc_distortion.hpp 
             ${CMAKE_CURRENT_SOURCE_DIR}/../../dependencies/dmetric/source/pcc_processing.hpp )

#
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/include/
                     ${CMAKE_CURRENT_SOURCE_DIR}/../../dependencies/
                     ${CMAKE_CURRENT_SOURCE_DIR}/../../dependencies/glad/include
                     ${CMAKE_CURRENT_SOURCE_DIR}/../../dependencies/eigen3
                     ${CMAKE_CURRENT_SOURCE_DIR}/../../dependencies/nanoflann
                     ${CMAKE_CURRENT_SOURCE_DIR}/../../deps/glfw/include)

ADD_LIBRARY( mmlib ${PCQM_SRC} 
                   ${GLAD_SRC} 
                   ${PCC_SRC} 
                   ${MM_LIB_SRC} 
                   ${MM_LIB_INC} )

target_link_libraries( mmlib glfw )


