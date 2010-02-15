if(DEFINED CMAKE_BUILD_TYPE)
   set(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE} CACHE STRING "Choose the type of
build, options are: None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug
Release RelWithDebInfo MinSizeRel.")
else()
   ##### Build types #################################################
   # single-configuration generator like Makefile generator creates following variables per default
   #
   # None (CMAKE_C_FLAGS or CMAKE_CXX_FLAGS used)
   # Debug (CMAKE_C_FLAGS_DEBUG or CMAKE_CXX_FLAGS_DEBUG)
   # Release (CMAKE_C_FLAGS_RELEASE or CMAKE_CXX_FLAGS_RELEASE)
   # RelWithDebInfo (CMAKE_C_FLAGS_RELWITHDEBINFO or CMAKE_CXX_FLAGS_RELWITHDEBINFO
   # MinSizeRel (CMAKE_C_FLAGS_MINSIZEREL or CMAKE_CXX_FLAGS_MINSIZEREL) 
   ####################################################################
   set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build,
options are: None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release
RelWithDebInfo MinSizeRel.")
endif()

message(STATUS "Build type set to: " ${CMAKE_BUILD_TYPE})

# Specifies a common place where CMake should put all the executables.
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
# Specifies a common place where CMake should put all the libraries
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

##### Add doxygen support ###################################################
include(FindDoxygen) #sets DOXYGEN_EXECUTABLE
if(DOXYGEN_EXECUTABLE)
    # uses
    # PROJECT_NAME           = @PROJECT_NAME@
    # PROJECT_NUMBER         = @PROJECT_VERSION@
    # OUTPUT_DIRECTORY       = @PROJECT_BINARY_DIR@/doc
    # INPUT                  = @PROJECT_SOURCE_DIR@/src
    # input output @ONLY: replace @VAR@ in the input file with the cmake variables
    CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/doc/Doxyfile.in ${PROJECT_SOURCE_DIR}/doc/Doxyfile @ONLY)
    # documentation can be generated with 'make doc'
    ADD_CUSTOM_TARGET(doc ${DOXYGEN_EXECUTABLE} ${PROJECT_SOURCE_DIR}/doc/Doxyfile)
    # generates documentation with cmake
    # QUIET mode is enabled in the configuration file (QUIET = YES)
    EXECUTE_PROCESS(COMMAND ${DOXYGEN_EXECUTABLE} ${PROJECT_SOURCE_DIR}/doc/Doxyfile )
endif(DOXYGEN_EXECUTABLE)
##### End doxygen support ###################################################

