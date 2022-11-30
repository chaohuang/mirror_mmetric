cmake_minimum_required(VERSION 3.17)

include( ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/cmake/CPM.cmake )

set( DIR ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/dmetric )
if( NOT EXISTS ${DIR} )
  CPMAddPackage( NAME             dmetric
                GIT_REPOSITORY    http://mpegx.int-evry.fr/software/MPEG/PCC/mpeg-pcc-dmetric.git
                GIT_TAG           release-v0.14
                SOURCE_DIR        ${DIR}
                DOWNLOAD_ONLY     YES )
endif()
