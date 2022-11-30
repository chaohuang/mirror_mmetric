cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

project(mm)

###################################
# input sources and headers 
###################################

file(GLOB MM_CMD_INC ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)
file(GLOB MM_CMD_SRC ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp)
file(GLOB MM_CMD_TST ${CMAKE_CURRENT_SOURCE_DIR}/../../test/*.sh)

# for nice display in visual solution
source_group("include\\" FILES ${MM_CMD_INC} )
source_group("source\\"  FILES ${MM_CMD_SRC} )
source_group("test\\"    FILES ${MM_CMD_TST} )

#
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/include/
                     ${CMAKE_CURRENT_SOURCE_DIR}/../lib/include/
                     ${MM_DEPS_DIR}/
                     ${MM_DEPS_DIR}/glad/include
                     ${MM_DEPS_DIR}/eigen3
                     ${MM_DEPS_DIR}/nanoflann
                     ${MM_DEPS_DIR}/glfw/include )

# 
add_executable(mm ${MM_CMD_SRC} ${MM_CMD_INC} ${MM_CMD_TST})

target_link_libraries(mm mmlib glfw)
#
install(TARGETS mm DESTINATION ${PROJECT_OUTPUT_FOLDER})

# NOTE: set a group policy?
# set_target_properties(mm PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ${PROJECT_OUTPUT_FOLDER})
# set_target_properties(mm PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${PROJECT_OUTPUT_FOLDER})
# set_target_properties(mm PROPERTIES DEBUG_POSTFIX "_d")
