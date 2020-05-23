# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.0)

project("libwsq" VERSION 1.0.0 DESCRIPTION "nist wsq routines")

if (MSVC)
    add_definitions(-w -D_WIN32_WINNT=0x600 -D_SCL_SECURE_NO_WARNINGS -D_WINSOCK_DEPRECATED_NO_WARNINGS)

    include_directories(
		include/
        d:/lib/vcpkg/installed/x64-windows/include/
    )

    link_directories(
        d:/lib/vcpkg/installed/x64-windows/lib/
    )
elseif(UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -w -Wall -Wno-long-long -Wno-deprecated-declarations -Wno-unknown-pragmas")

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

add_library(${PROJECT_NAME} STATIC ${WSQ_SOURCE})

set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER include/wsq.h)

target_link_libraries(${PROJECT_NAME}
    ${JPEG_LIBRARIES}
    ${PNG_LIBRARIES}
    ${ZLIB_LIBRARIES}
)

install(TARGETS ${PROJECT_NAME}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
