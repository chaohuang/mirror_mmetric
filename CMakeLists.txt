cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

set(PROJECT_OUTPUT_FOLDER "${CMAKE_CURRENT_SOURCE_DIR}/bin")
set(CMAKE_CONFIGURATION_TYPES "Release;Debug" CACHE STRING "Release;Debug")

project(mm)

option(USE_OPENMP              "Use openmp libraries if available"      ON)
option(MM_BUILD_CMD            "Build mm software application"          ON)

# followjng reuqires/activates cxx17 
set(CMAKE_CXX_STANDARD          17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS        OFF)

###################################
# per-system compiler/linker config 
###################################

# openMP 
if (USE_OPENMP)
	find_package(OpenMP)
	if (OPENMP_FOUND)
	  message(STATUS "OpenMP found.")
	  set (CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   ${OpenMP_C_FLAGS}")
	  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
	  add_compile_options( -D OPENMP_FOUND ) 
	else()
		message( STATUS "Could not find openMP, disable openMP")
		SET( USE_OPENMP FALSE)
	endif()
endif()

# dependencies
include( ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/cmake/glfw.cmake )
include( ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/cmake/dmetric.cmake )

# Visual studio
if ( MSVC )    
	# Release compil and link config (/fp:strict is important for portable results)
	set(CMAKE_CXX_FLAGS_RELEASE "/DNDEBUG /DNOMINMAX /fp:strict /bigobj /Ox /Ob2 /Oi /Ot /Oy /GF /FD /EHsc /MD /Gy /W3 /MP /nologo /c /TP /errorReport:prompt")
	set(CMAKE_C_FLAGS_RELEASE   "/DNDEBUG /DNOMINMAX /fp:strict /bigobj /Ox /Ob2 /Oi /Ot /Oy /GF /FD /EHsc /MD /Gy /W3 /MP /nologo /c /TC /errorReport:prompt")

	# Debug compil and link
	set(CMAKE_CXX_FLAGS_DEBUG "/DDEBUG /DNOMINMAX /fp:strict /bigobj /Od /GF /FD /EHsc /MD /Gy /W3 /MP /nologo /c /Zi /TP /errorReport:prompt")
	set(CMAKE_C_FLAGS_DEBUG   "/DDEBUG /DNOMINMAX /fp:strict /bigobj /Od /GF /FD /EHsc /MD /Gy /W3 /MP /nologo /c /Zi /TC /errorReport:prompt")  

	# 32 bits large memory adresses (Removes 1GB limitation, handle up to 4GB)
	set(CMAKE_EXE_LINKER_FLAGS    "/LARGEADDRESSAWARE /INCREMENTAL:NO /NOLOGO")
	set(CMAKE_SHARED_LINKER_FLAGS "/LARGEADDRESSAWARE /INCREMENTAL:NO /NOLOGO")

	set(CMAKE_EXE_LINKER_FLAGS_RELEASE    "/OPT:REF /OPT:ICF /FIXED:No")
	set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "/OPT:REF /OPT:ICF /FIXED:No")

	set(CMAKE_EXE_LINKER_FLAGS_DEBUG    "/DEBUG")
	set(CMAKE_SHARED_LINKER_FLAGS_DEBUG "/DEBUG")

else ( ) # all others GCC, CLang (Linux, MacOS), etc... 
	# Compil and link config (no-fast-math is important for portable results)
	set(CMAKE_CXX_FLAGS_RELEASE "-Wno-unused-result -O3 -DNDEBUG -D_REENTRANT -DLinux -fno-fast-math -ftree-vectorize -msse2 -msse3 -D__STDC_CONSTANT_MACROS -fpermissive")
	set(CMAKE_C_FLAGS_RELEASE   "-Wno-unused-result -O3 -DNDEBUG -D_REENTRANT -DLinux -fno-fast-math -ftree-vectorize -msse2 -msse3")
	
	set(CMAKE_CXX_FLAGS_DEBUG   "-Wno-unused-result -g -DDEBUG  -D_REENTRANT -DLinux -D__STDC_CONSTANT_MACROS -fno-fast-math  -fpermissive")
	set(CMAKE_C_FLAGS_DEBUG     "-Wno-unused-result -g -DDEBUG  -D_REENTRANT -DLinux")

	if( NOT USE_OPENMP)
		set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
	endif()
  if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
		if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 4.7 OR CMAKE_CXX_COMPILER_VERSION VERSION_EQUAL 4.7)
			message( "gcc version greater or equal to 4.7 (CMAKE_CXX_COMPILER_VERSION = ${CMAKE_CXX_COMPILER_VERSION})" )
			set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 ")
		else ()
			message( "gcc version lower than 4.7 (CMAKE_CXX_COMPILER_VERSION = ${CMAKE_CXX_COMPILER_VERSION})" )
			set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
		endif ()
	endif()
endif()

SET( MM_DEPS_DIR     ${CMAKE_CURRENT_SOURCE_DIR}/dependencies )

SET( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib )
SET( CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib )
SET( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin )

include( dependencies/cmake/clang-format.cmake )
#
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/source/lib)
if( ${MM_BUILD_CMD} )
  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/source/cmd)
endif()

