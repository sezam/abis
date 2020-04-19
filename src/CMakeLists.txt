# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.0)

if (MSVC)
  add_definitions(-D_WIN32_WINNT=0x600 -D_SCL_SECURE_NO_WARNINGS -D_WINSOCK_DEPRECATED_NO_WARNINGS)
endif()

file (GLOB COMMON_SRC *.cpp *.h finger/*.h)
set (SOURCE  ${COMMON_SRC})

include_directories(
    d:/lib/vcpkg/installed/x64-windows/include/
)

link_directories(
    d:/lib/vcpkg/installed/x64-windows/lib/
)

find_package(ZLIB REQUIRED)
find_package(Boost REQUIRED)
find_package(cpprestsdk REQUIRED)

#Финальный штрих и сборка.
add_executable(${PACKAGE_TARNAME} ${SOURCE}) 
target_link_libraries(${PACKAGE_TARNAME} 
    cpprest_2_10.lib 
    boost_locale-vc140-mt.lib
    boost_date_time-vc140-mt.lib
    boost_regex-vc140-mt.lib
    jpeg.lib 
    libpng16.lib
)