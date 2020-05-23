# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.0)

set(PACKAGE_NAME      "abis-rest-server")
set(PACKAGE_VERSION   "0.3")
set(PACKAGE_STRING    "${PACKAGE_NAME} ${PACKAGE_VERSION}")
set(PACKAGE_TARNAME   "${PACKAGE_NAME}-${PACKAGE_VERSION}")

project(${PACKAGE_NAME} VERSION ${PACKAGE_VERSION} DESCRIPTION "abis-rest-server" LANGUAGES CXX)

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 7.1)
        message(FATAL_ERROR "GCC version must be at least 7.1!")
    endif()
endif()

add_subdirectory (nbis)
add_subdirectory (src)