#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE "test_pkgconfig_helper"
#define BOOST_AUTO_TEST_MAIN

#include <boost/bind.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/test/execution_monitor.hpp>

#include <orocos_cpp/PkgConfigHelper.hpp>
#include <stdlib.h>

using namespace orocos_cpp;

std::vector< std::pair<std::string,std::string> > make_expected_properties(){
    return {
        std::pair<std::string,std::string>("Name","aggregatorTypekit"),
        std::pair<std::string,std::string>("Version", "0.0"),
        std::pair<std::string,std::string>("Requires", "aggregator"),
        std::pair<std::string,std::string>("Description", "aggregator types support for the Orocos type system"),
        std::pair<std::string,std::string>("Libs", "-L/media/wirkus/Data/development/rock-runtime/install/lib/orocos/types -laggregator-typekit-gnulinux"),
        std::pair<std::string,std::string>("Cflags", "-I/media/wirkus/Data/development/rock-runtime/install/include/orocos -I/media/wirkus/Data/development/rock-runtime/install/include/orocos/aggregator/types -I/media/wirkus/Data/development/rock-runtime/install/include \"-DOROCOS_TARGET=gnulinux\" \"-I/media/wirkus/Data/development/rock-runtime/install/include\" \"-I/usr/include/eigen3\" \"-I/media/wirkus/Data/development/rock-runtime/install/include/orocos\" \"-I/media/wirkus/Data/development/rock-runtime/install/include/orocos/base/types\" \"-I/media/wirkus/Data/development/rock-runtime/install/include/orocos/std/types\"")
    };
}

std::vector< std::pair<std::string,std::string> > make_expected_variables(){
    return {
            std::pair<std::string,std::string>("prefix","/media/wirkus/Data/development/rock-runtime/install"),
            std::pair<std::string,std::string>("exec_prefix", "/media/wirkus/Data/development/rock-runtime/install"),
            std::pair<std::string,std::string>("libdir", "/media/wirkus/Data/development/rock-runtime/install/lib/orocos/types"),
            std::pair<std::string,std::string>("includedir", "/media/wirkus/Data/development/rock-runtime/install/include/orocos"),
            std::pair<std::string,std::string>("project_name", "aggregator"),
            std::pair<std::string,std::string>("deffile", "/media/wirkus/Data/development/rock-runtime/install/share/orogen/aggregator.orogen"),
            std::pair<std::string,std::string>("type_registry", "/media/wirkus/Data/development/rock-runtime/install/share/orogen/aggregator.tlb"),
        };
}



BOOST_AUTO_TEST_CASE(parsePkgConfig)
{
    std::vector< std::pair<std::string,std::string> > expected_properties = make_expected_properties();
    std::vector< std::pair<std::string,std::string> > expected_variables = make_expected_variables();


    int ret = putenv("PKG_CONFIG_PATH=../../test/test_pkgconfig");
    std::map<std::string,std::string> variables;
    std::map<std::string,std::string> properties;
    bool st = PkgConfigHelper::parsePkgConfig("aggregator-typekit-gnulinux.pc", variables, properties, false);
    BOOST_CHECK(st);

    BOOST_ASSERT(variables.size() == expected_variables.size());

    for(size_t i=0; i<variables.size(); i++){
        std::pair<std::string,std::string> expected = expected_variables[i];
        std::map<std::string,std::string>::iterator it = variables.find(expected.first);
        BOOST_ASSERT(it != variables.end());
        BOOST_CHECK_EQUAL(it->second, expected.second);
    }

    BOOST_ASSERT(properties.size() == expected_properties.size());
    for(size_t i=0; i<properties.size(); i++){
        std::pair<std::string,std::string> expected = expected_properties[i];
        std::map<std::string,std::string>::iterator it = properties.find(expected.first);
        BOOST_ASSERT(it != properties.end());
        BOOST_CHECK_EQUAL(it->second, expected.second);
    }

    BOOST_CHECK(st);
}

BOOST_AUTO_TEST_CASE(oldParsePkgConfigStillWorking)
{
    std::vector< std::pair<std::string,std::string> > expected_properties = make_expected_properties();
    std::vector< std::pair<std::string,std::string> > expected_variables = make_expected_variables();

    int ret = putenv("PKG_CONFIG_PATH=../../test/test_pkgconfig");
    std::vector<std::string> fields={"type_registry"};
    std::vector<std::string> res;

    bool st = PkgConfigHelper::parsePkgConfig("aggregator-typekit-gnulinux.pc", fields, res, false);
    BOOST_CHECK(st);

    BOOST_ASSERT(fields.size() == res.size());
    BOOST_ASSERT(res.size() == 1);

    fields={"gibt'snicht"};
    st = PkgConfigHelper::parsePkgConfig("aggregator-typekit-gnulinux.pc", fields, res, false);
    BOOST_CHECK(!st);


    fields={"Description", "Libs"};
    st = PkgConfigHelper::parsePkgConfig("aggregator-typekit-gnulinux.pc", fields, res, true);
    BOOST_CHECK(st);

    BOOST_ASSERT(res.size() == 2);
}
