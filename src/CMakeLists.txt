find_package( Boost COMPONENTS thread system filesystem)
rock_library(orocos_cpp
    SOURCES 
        Bundle.cpp
        ConfigurationHelper.cpp
        TransformerHelper.cpp
        LoggingHelper.cpp
        Spawner.cpp
        NameService.cpp
        CorbaNameService.cpp
        Deployment.cpp
    HEADERS 
        Bundle.hpp
        ConfigurationHelper.hpp
        TransformerHelper.hpp
        LoggingHelper.hpp
        Spawner.hpp
        NameService.hpp
        CorbaNameService.hpp
        Deployment.hpp
    DEPS_PKGCONFIG
        orocos_cpp_base
        rtt_typelib-${OROCOS_TARGET}
        orocos-rtt-${OROCOS_TARGET}
        orocos-rtt-mqueue-${OROCOS_TARGET}
        orocos-rtt-corba-${OROCOS_TARGET}
        typelib
        yaml-cpp
        transformer
        smurf
        logger-proxies
    DEPS_PLAIN 
        Boost_SYSTEM Boost_FILESYSTEM Boost_THREAD
    )

rock_executable(listAll Main.cpp
    DEPS orocos_cpp)

rock_executable(proxyTest main2.cpp
    DEPS orocos_cpp)

rock_executable(config main3.cpp
    DEPS orocos_cpp)

rock_executable(nameservice main4.cpp
    DEPS orocos_cpp)

# target_link_libraries(listAll rtt-typekit-gnulinux)

# orogen_pkg_check_modules(base_TYPEKIT REQUIRED base-typekit-gnulinux)
# include_directories(${base_TYPEKIT_INCLUDE_DIRS})
# add_definitions(${base_TYPEKIT_CFLAGS_OTHER})
# link_directories(${base_TYPEKIT_LIBRARY_DIRS})
# orogen_pkg_check_modules(std_TYPEKIT REQUIRED std-typekit-gnulinux)
# include_directories(${std_TYPEKIT_INCLUDE_DIRS})
# add_definitions(${std_TYPEKIT_CFLAGS_OTHER})
# link_directories(${std_TYPEKIT_LIBRARY_DIRS})
