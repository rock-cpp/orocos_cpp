rock_library(orocos_cpp
    SOURCES 
        corba.cc
        corba_name_service_client.cc
    HEADERS corba.hh
    DEPS_PKGCONFIG
        rtt_typelib-${OROCOS_TARGET}
        orocos-rtt-${OROCOS_TARGET}
        orocos-rtt-mqueue-${OROCOS_TARGET}
        orocos-rtt-corba-${OROCOS_TARGET}
    )

rock_executable(listAll Main.cpp
    DEPS orocos_cpp)

