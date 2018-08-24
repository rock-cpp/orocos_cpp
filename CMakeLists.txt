# CMakeLists.txt has to be located in the project folder and cmake has to be
# executed from 'project/build' with 'cmake ../'.
cmake_minimum_required(VERSION 2.6)
find_package(Rock)
rock_init(orocos_cpp 0.1)

SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -std=c++0x" )

set(OROCOS_TARGET "gnulinux")
rock_standard_layout()

add_library(test_typeregistry SHARED test/test_typeregistry.cpp)
