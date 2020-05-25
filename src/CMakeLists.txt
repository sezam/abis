# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.0)

add_compile_options(
    -w
    -D_SCL_SECURE_NO_WARNINGS 
    -D_WINSOCK_DEPRECATED_NO_WARNINGS 
    -DBOOST_UUID_FORCE_AUTO_LINK
#    -Wall 
#    -Wno-long-long 
#    -Wno-deprecated-declarations 
#    -Wno-unknown-pragmas
)

if(UNIX)
    include_directories(
        /opt/vcpkg/installed/x64-linux/include/postgresql/
        /opt/vcpkg/installed/x64-linux/include/postgresql/server/
        /opt/vcpkg/installed/x64-linux/include/postgresql/internal/
    )

    find_library(FPRINT_LIB libfprint.so ${PROJECT_SOURCE_DIR}/shared/)
    if(FPRINT_LIB)
        message(STATUS "Found libfprint: ${FPRINT_LIB}")
    else()
        message(FATAL_ERROR "Looking for libfprint - not found")
    endif()

    find_library(LIVE_LIB libLive.so ${PROJECT_SOURCE_DIR}/shared/)
    if(LIVE_LIB)
        message(STATUS "Found libLive: ${LIVE_LIB}")
    else()
        message(FATAL_ERROR "Looking for libLive - not found")
    endif()
endif()

file (GLOB COMMON_SRC *.cpp *.h finger/*.h live/*.h)
set (SOURCE  ${COMMON_SRC})

find_package(Boost 1.72.0 REQUIRED COMPONENTS log program_options thread)
find_package(cpprestsdk REQUIRED NAMES cpprestsdk cpprest)
find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(ZLIB REQUIRED)
find_package(PostgreSQL REQUIRED)
find_package(OpenCV REQUIRED COMPONENTS core imgcodecs)
find_library(libwsq REQUIRED)

#Финальный штрих и сборка.
add_executable(${PACKAGE_TARNAME} ${SOURCE})

target_link_libraries(${PACKAGE_TARNAME}
    cpprestsdk::cpprest
    Boost::log 
    Boost::program_options 
    Boost::thread 
    ${OpenSSL_LIBRARIES}
    ${ZLIB_LIBRARIES}
    PostgreSQL::PostgreSQL
    opencv_core
    opencv_imgcodecs
	libwsq
)

if(UNIX)
    target_link_libraries(${PACKAGE_TARNAME}
        ${FPRINT_LIB}
        ${LIVE_LIB}
    )
endif()