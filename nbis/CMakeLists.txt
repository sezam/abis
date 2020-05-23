# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.0)

project("libwsq" VERSION 1.0.0 DESCRIPTION "nist wsq routines" LANGUAGES C)

include_directories(
    include/
)

add_compile_options(
    -w
)

file (GLOB WSQ_COMMON_SRC fet/*.cpp ioutil/*.cpp jpegl/*.cpp util/*.cpp wsq/*.cpp)
set (WSQ_SOURCE  ${WSQ_COMMON_SRC})

find_package(JPEG REQUIRED)
find_package(PNG REQUIRED)
find_package(ZLIB REQUIRED)

add_library(${PROJECT_NAME} STATIC ${WSQ_SOURCE})

target_include_directories(${PROJECT_NAME} INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(${PROJECT_NAME}
    ${JPEG_LIBRARIES}
    ${PNG_LIBRARIES}
    ${ZLIB_LIBRARIES}
)

install(TARGETS ${PROJECT_NAME}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

#install(DIRECTORY include/${PROJECT_NAME} DESTINATION include)
