# CMakeLists.txt has to be located in the project folder and cmake has to be
# executed from 'project/build' with 'cmake ../'.

cmake_minimum_required(VERSION 2.6)

project(dummyproject)
set(PROJECT_VERSION 1.0)
set(PROJECT_DESCRIPTION "Dummy project just to show how the infrastructure is working")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")
include(CommonSetup)

# Include headers within source
# If you create subdirectories within source include headers with subdirectory, i.e. 
# #include "subdirectory/myheader.h"
include_directories(${PROJECT_SOURCE_DIR}/external/include)
include_directories(${PROJECT_SOURCE_DIR}/src)

# command line output on, you can also use make VERBOSE=1
# set(CMAKE_VERBOSE_MAKEFILE on)

### Uncomment if you want to use Qt
# set(QT_SEARCH_PATH /opt)
# find_package(Qt4 4.5 REQUIRED QtTest QtThread QtXml QtNetwork QtGui)
# include(${QT_USE_FILES})

### Uncomment if you want to use Boost. Use the first find_package stanza to
### search for headers only, and the second one if you need binary libraries
#set(Boost_ADDITIONAL_VERSION 1.40 1.41)
#find_package(Boost REQUIRED)
#find_package(Boost REQUIRED COMPONENTS graph filesystem)
#include_directories(${Boost_INCLUDE_DIRS})

### Update external libraries that are available
set(EXTERNAL_LIBS ${Boost_LIBRARIES} ${QT_LIBRARIES})

# Process CMakeLists.txt in the following subdirectory
add_subdirectory(src)


