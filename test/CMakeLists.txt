rock_testsuite(test_typeregistry test_typeregistry.cpp
    DEPS orocos_cpp
    DEPS_PKGCONFIG base-types orocos-rtt-${OROCOS_TARGET})

rock_testsuite(test_pkgconfig_helper test_pkgconfig_helper.cpp
    DEPS orocos_cpp
    DEPS_PKGCONFIG base-types orocos-rtt-${OROCOS_TARGET})


configure_file(${CMAKE_CURRENT_SOURCE_DIR}/testfile.tlb
            ${CMAKE_CURRENT_BINARY_DIR}/testfile.tlb COPYONLY)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/testfile.typelist
    ${CMAKE_CURRENT_BINARY_DIR}/testfile.typelist COPYONLY)
