# CMakeLists.txt has to be located in the project folder and cmake has to be
# executed from 'project/build' with 'cmake ../'.

cmake_minimum_required(VERSION 2.6)

project(dummyproject)
set(PROJECT_VERSION 1.0)
set(PROJECT_DESCRIPTION "Dummy project just to show how the infrastructure is working")

SET (CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
USE(CommonSetup)

# Include headers within source
# If you create subdirectories within source include headers with subdirectory, i.e. 
# #include "subdirectory/myheader.h"
include_directories(${PROJECT_SOURCE_DIR}/src)

# command line output on, you can also use make VERBOSE=1
# set(CMAKE_VERBOSE_MAKEFILE on)

### Uncomment if you want to use Qt
# set(QT_SEARCH_PATH /opt)
# find_package(QT4 4.5 REQUIRED QtTest QtThread QtXml QtNetwork QtGui)
# include(${QT_USE_FILES})

### Uncomment if you want to use Boost. Use the first find_package stanza to
### search for headers only, and the second one if you need binary libraries
# set(Boost_ADDITIONAL_VERSION 1.40 1.41)
# find_package(Boost REQUIRED)
# # find_package(Boost REQUIRED COMPONENTS graph)
# include_directories(${Boost_INCLUDE_DIRS})

# Process CMakeLists.txt in the following subdirectory
add_subdirectory(src)

##### Common installation #################################################
# COPY Configuration files into build directory. Workaround: remove the .pc.in
# file, as we don't want to install it
execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/configuration ${PROJECT_BINARY_DIR}/configuration)
execute_process(COMMAND ${CMAKE_COMMAND} -E remove -f ${PROJECT_BINARY_DIR}/configuration/${PROJECT_NAME}.pc.in)

# Install the scripts
install(DIRECTORY ${PROJECT_SOURCE_DIR}/scripts/ DESTINATION scripts)

# Generate and install the pkg-config file
configure_file(${PROJECT_SOURCE_DIR}/configuration/${PROJECT_NAME}.pc.in
		${PROJECT_BINARY_DIR}/configuration/${PROJECT_NAME}.pc @ONLY)
install(FILES ${CMAKE_BINARY_DIR}/configuration/${PROJECT_NAME}.pc DESTINATION lib/pkgconfig)

