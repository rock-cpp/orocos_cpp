# CMakeLists.txt has to be located in the project folder and cmake has to be
# executed from 'project/build' with 'cmake ../'.
cmake_minimum_required(VERSION 2.6)
find_package(Rock)
rock_init(orocos_cpp 0.1)

include(FindPkgConfig)

set(OROCOS_TARGET "gnulinux")

pkg_check_modules(RTT_Typelib REQUIRED rtt_typelib-${OROCOS_TARGET})
include_directories( ${RTT_Typelib_INCLUDE_DIRS} )
link_directories( ${RTT_Typelib_LIBRARY_DIRS} )

pkg_check_modules(RTT REQUIRED orocos-rtt-${OROCOS_TARGET})
ADD_DEFINITIONS( ${RTT_CFLAGS} )
INCLUDE_DIRECTORIES( ${RTT_INCLUDE_DIRS} )
LINK_DIRECTORIES( ${RTT_LIBRARY_DIRS} )

pkg_check_modules(RTT_MQUEUE orocos-rtt-mqueue-${OROCOS_TARGET})
if (RTT_MQUEUE_FOUND)
    include_directories( ${RTT_MQUEUE_INCLUDE_DIRS} )
    add_definitions(-DHAS_MQUEUE)
    list(APPEND ADDITIONAL_RTT_PLUGINS rtt-transport-mqueue)
else()
    message(STATUS "could not find the RTT MQueue transport, disabling support in orocos.rb")
endif()

pkg_check_modules(RTT_ROS orocos-rtt-ros-${OROCOS_TARGET} roscpp)
if (RTT_ROS_FOUND)
    include_directories( ${RTT_ROS_INCLUDE_DIRS} )
    add_definitions(-DHAS_ROS)
    list(APPEND ADDITIONAL_RTT_PLUGINS rtt-transport-ros)
    list(APPEND ROS_FILES ros.cc)
else()
    message(STATUS "could not find the RTT ROS transport, disabling support in orocos.rb")
endif()

pkg_check_modules(RTT_CORBA REQUIRED orocos-rtt-corba-${OROCOS_TARGET})
FIND_PATH(IDL_DIR TaskContext.idl ${RTT_CORBA_INCLUDE_DIRS})
if (NOT IDL_DIR)
    message(FATAL_ERROR "cannot find the Orocos/CORBA IDL files")
endif()
FILE(GLOB IDL_FILES ${IDL_DIR}/*.idl)

pkg_check_modules(TYPELIB REQUIRED typelib)
INCLUDE_DIRECTORIES( ${TYPELIB_INCLUDE_DIRS} )
LINK_DIRECTORIES( ${TYPELIB_LIBRARY_DIRS} )

list(APPEND CMAKE_PREFIX_PATH ${RTT_PREFIX})
find_package(RTTPlugin COMPONENTS rtt-typekit rtt-transport-corba ${ADDITIONAL_RTT_PLUGINS})

ADD_DEFINITIONS( -DOROCOS_TARGET=${OROCOS_TARGET} )
INCLUDE_DIRECTORIES( ${RTT_CORBA_INCLUDE_DIRS} )
LINK_DIRECTORIES( ${RTT_CORBA_LIBRARY_DIRS} )

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

MACRO(OMNIORB)
    ADD_DEFINITIONS(-DCORBA_IS_OMNIORB)
    FOREACH(idl_file ${IDL_FILES})
        FILE(RELATIVE_PATH idl_basename ${IDL_DIR} ${idl_file})

        STRING(REGEX REPLACE ".idl$" "C.h"   hdr_file ${idl_basename})
        STRING(REGEX REPLACE ".idl$" "DynSK.cpp" dyn_file ${idl_basename})
        STRING(REGEX REPLACE ".idl$" "C.cpp" cpp_file ${idl_basename})
        SET(client_hdr "${hdr_file}")
        SET(client_imp "${cpp_file};${dyn_file}")

        LIST(APPEND ORB_IDL_FILES ${client_imp})
        add_custom_command(OUTPUT ${client_hdr} ${client_imp}
            COMMAND omniidl -bcxx -Wba -Wbh=C.h -Wbs=C.cpp -Wbd=DynSK.cpp -I${CMAKE_CURRENT_SOURCE_DIR} ${idl_file}
            DEPENDS ${idl_file})
    ENDFOREACH(idl_file ${IDL_FILES})

    SET(ORB_LIBRARIES omniORB4 omniDynamic4)
ENDMACRO(OMNIORB)

link_directories(${RTT_ROS_LIBRARY_DIRS})

OMNIORB()

rock_standard_layout()
