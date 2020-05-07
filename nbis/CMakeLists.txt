# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.0)

if (MSVC)
    add_definitions(-D_WIN32_WINNT=0x600 -D_SCL_SECURE_NO_WARNINGS -D_WINSOCK_DEPRECATED_NO_WARNINGS)

    include_directories(
		include/
        d:/lib/vcpkg/installed/x64-windows/include/
    )

    link_directories(
        d:/lib/vcpkg/installed/x64-windows/lib/
    )
elseif(UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall -Wno-long-long -Wno-deprecated-declarations -Wno-unknown-pragmas")

    include_directories(
		include/
        /opt/vcpkg/installed/x64-linux/include/
    )

    link_directories(
        /opt/vcpkg/installed/x64-linux/lib
    )
endif()

file (GLOB WSQ_COMMON_SRC fet/*.cpp ioutil/*.cpp jpegl/*.cpp util/*.cpp wsq/*.cpp)
set (WSQ_SOURCE  ${WSQ_COMMON_SRC})

find_package(JPEG REQUIRED)
find_package(PNG REQUIRED)
find_package(ZLIB REQUIRED)

add_library(abiswsq STATIC ${WSQ_SOURCE})

target_link_libraries(abiswsq
    ${JPEG_LIBRARIES}
    ${PNG_LIBRARIES}
    ${ZLIB_LIBRARIES}
)
