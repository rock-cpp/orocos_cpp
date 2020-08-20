find_package( Boost COMPONENTS thread system filesystem regex)
rock_library(orocos_cpp
    SOURCES 
        ConfigurationHelper.cpp
        TypeRegistry.cpp
        LoggingHelper.cpp
        LoggerProxy.cpp
        Spawner.cpp
        NameService.cpp
        CorbaNameService.cpp
        Deployment.cpp
        PkgConfigHelper.cpp
        PluginHelper.cpp
        PkgConfigRegistry.cpp
        orocos_cpp.cpp
        OrocosCppConfig.hpp
    HEADERS 
        ConfigurationHelper.hpp
        TypeRegistry.hpp
        LoggingHelper.hpp
        Spawner.hpp
        NameService.hpp
        CorbaNameService.hpp
        Deployment.hpp
        PkgConfigHelper.hpp
        PluginHelper.hpp
        PkgConfigRegistry.hpp
        orocos_cpp.hpp
        OrocosCppConfig.hpp
    DEPS_PKGCONFIG
        orocos_cpp_base
        rtt_typelib-${OROCOS_TARGET}
        orocos-rtt-${OROCOS_TARGET}
        orocos-rtt-mqueue-${OROCOS_TARGET}
        orocos-rtt-corba-${OROCOS_TARGET}
        typelib
        lib_config
        backward
    DEPS_PLAIN 
        Boost_SYSTEM Boost_FILESYSTEM Boost_REGEX Boost_THREAD
    )

rock_executable(listAll Main.cpp
    DEPS orocos_cpp
    NOINSTALL)

rock_executable(proxyTest main2.cpp
    DEPS orocos_cpp
    NOINSTALL)

rock_executable(config main3.cpp
    DEPS orocos_cpp
    NOINSTALL)

rock_executable(nameservice main4.cpp
    DEPS orocos_cpp
    NOINSTALL)
