cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

set(PROJECT_OUTPUT_FOLDER "${CMAKE_CURRENT_SOURCE_DIR}/bin")
set(CMAKE_CONFIGURATION_TYPES "Release;Debug" CACHE STRING "Release;Debug")

project(mm)

option(USE_OPENMP      "Use openmp libraries if available" ON)

###################################
# per-system compiler/linker config 
###################################

if (USE_OPENMP)
	find_package(OpenMP)
	if (OPENMP_FOUND)
	  message(STATUS "OpenMP found.")
	  set (CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   ${OpenMP_C_FLAGS}")
	  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
	  add_compile_options( -D OPENMP_FOUND ) 
	endif()
endif()

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
	
# Mac
elseif ( APPLE )

	# todo

# General GNU
elseif ( UNIX ) # all others Linux, etc...

    # Compil and link config (no-fast-math is important for portable results)
    set(CMAKE_CXX_FLAGS_RELEASE "-Wno-unused-result -O3 -DNDEBUG -D_REENTRANT -DLinux -fno-fast-math -ftree-vectorize -msse2 -msse3 -D__STDC_CONSTANT_MACROS -fpermissive")
    set(CMAKE_C_FLAGS_RELEASE   "-Wno-unused-result -O3 -DNDEBUG -D_REENTRANT -DLinux -fno-fast-math -ftree-vectorize -msse2 -msse3")
    
    set(CMAKE_CXX_FLAGS_DEBUG   "-Wno-unused-result -g -DDEBUG  -D_REENTRANT -DLinux -D__STDC_CONSTANT_MACROS -fno-fast-math  -fpermissive")
    set(CMAKE_C_FLAGS_DEBUG     "-Wno-unused-result -g -DDEBUG  -D_REENTRANT -DLinux")
	
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 4.7 OR CMAKE_CXX_COMPILER_VERSION VERSION_EQUAL 4.7)
        message( "gcc version greater or equal to 4.7" )
		set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
	else ()
		message( "gcc version lower than 4.7" )
		set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
	endif ()

endif()

###################################
# input sources and headers 
###################################

file(GLOB SLN_INC ${CMAKE_CURRENT_SOURCE_DIR}/source/*.h)
file(GLOB SLN_SRC ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp)
file(GLOB SLN_TST ${CMAKE_CURRENT_SOURCE_DIR}/test/*.sh)

set(PCQM_SRC ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/pcqm/pcqm.cpp )
set(PCQM_INC 
	${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/pcqm/pcqm.h 
	${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/pcqm/PointSet.h
	${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/pcqm/resources.h
	${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/pcqm/utilities.h )
set(PCC_SRC 
	${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/pcc/pcc_distortion.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/pcc/pcc_processing.cpp )
set(PCC_INC
	${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/pcc/pcc_distortion.hpp 
	${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/pcc/pcc_processing.hpp )

# for nice display in visual solution
source_group("3rdparty\\" FILES ${GLAD_SRC})
source_group("3rdparty\\pcqm" FILES ${PCQM_SRC} ${PCQM_INC})
source_group("3rdparty\\pcc" FILES ${PCC_SRC} ${PCC_INC})
source_group("source\\" FILES ${SLN_SRC} ${SLN_INC})
source_group("test\\" FILES ${SLN_TST})

#
include_directories( 
	${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/
	${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/eigen3
	${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/nanoflann)

add_executable(mm ${PCQM_SRC} ${GLAD_SRC} ${PCC_SRC} ${SLN_SRC} ${SLN_INC} ${SLN_TST})

#
install(TARGETS mm DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/bin)

set_target_properties(mm PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ${PROJECT_OUTPUT_FOLDER})
set_target_properties(mm PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${PROJECT_OUTPUT_FOLDER})
set_target_properties(mm PROPERTIES DEBUG_POSTFIX "_d")

