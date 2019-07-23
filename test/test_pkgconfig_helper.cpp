#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE "test_pkgconfig_helper"
#define BOOST_AUTO_TEST_MAIN

#include <boost/bind.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/test/execution_monitor.hpp>

#include <orocos_cpp/PkgConfigHelper.hpp>
#include <stdlib.h>

using namespace orocos_cpp;

BOOST_AUTO_TEST_CASE(parsePkgConfig)
{
    std::vector< std::pair<std::string,std::string> > expected_properties = {
        std::pair<std::string,std::string>("Name","testTypekit"),
        std::pair<std::string,std::string>("Version", "0.0"),
        std::pair<std::string,std::string>("Requires", "test"),
        std::pair<std::string,std::string>("Description", "test types support for the Orocos type system"),
        std::pair<std::string,std::string>("Libs", "-L/my/prefix/lib/orocos/types -ltest-typekit-gnulinux"),
        std::pair<std::string,std::string>("Cflags", "-I/my/prefix/include/orocos -I/my/prefix/include/orocos/test/types \"-DOROCOS_TARGET=gnulinux\"")
    };
    std::vector< std::pair<std::string,std::string> > expected_variables = {
        std::pair<std::string,std::string>("prefix","/my/prefix"),
        std::pair<std::string,std::string>("exec_prefix", "/my/prefix"),
        std::pair<std::string,std::string>("libdir", "/my/prefix/lib/orocos/types"),
        std::pair<std::string,std::string>("includedir", "/my/prefix/include/orocos"),
        std::pair<std::string,std::string>("project_name", "test"),
        std::pair<std::string,std::string>("deffile", "/my/prefix/share/orogen/test.orogen"),
        std::pair<std::string,std::string>("type_registry", "/my/prefix/share/orogen/test.tlb"),
        std::pair<std::string,std::string>("typekits", "base test std"),
        std::pair<std::string,std::string>("deployed_tasks", "consumer,producer,relay"),
        std::pair<std::string,std::string>("deployed_tasks_with_models", "consumer,my::Consumer,producer,my::Producer"),
    };


    int ret = putenv("PKG_CONFIG_PATH=../../test/test_pkgconfig");
    std::map<std::string,std::string> variables;
    std::map<std::string,std::string> properties;
    bool st = PkgConfigHelper::parsePkgConfig("test-typekit-gnulinux.pc", variables, properties, false);
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
    std::vector< std::pair<std::string,std::string> > expected_properties = {
        std::pair<std::string,std::string>("Description", "test types support for the Orocos type system"),
        std::pair<std::string,std::string>("Libs", "-L/my/prefix/lib/orocos/types -ltest-typekit-gnulinux")
    };
    std::vector< std::pair<std::string,std::string> > expected_variables = {
        std::pair<std::string,std::string>("type_registry", "/my/prefix/share/orogen/test.tlb"),
        std::pair<std::string,std::string>("deployed_tasks", "consumer,producer,relay"),
        std::pair<std::string,std::string>("deployed_tasks_with_models", "consumer,my::Consumer,producer,my::Producer")
    };

    int ret = putenv("PKG_CONFIG_PATH=../../test/test_pkgconfig");
    std::vector<std::string> fields={"type_registry", "deployed_tasks", "deployed_tasks_with_models"};
    std::vector<std::string> res;

    bool st = PkgConfigHelper::parsePkgConfig("test-typekit-gnulinux.pc", fields, res, false);
    BOOST_CHECK(st);

    BOOST_ASSERT(fields.size() == res.size());
    BOOST_ASSERT(res.size() == expected_variables.size());

    for(size_t i=0; i<fields.size(); i++){
        std::pair<std::string,std::string> expected = expected_variables[i];
        std::string val = res[i];
        BOOST_CHECK_EQUAL(val, expected.second);
    }

    fields={"gibt'snicht"};
    st = PkgConfigHelper::parsePkgConfig("test-typekit-gnulinux.pc", fields, res, false);
    BOOST_CHECK(!st);


    fields={"Description", "Libs"};
    st = PkgConfigHelper::parsePkgConfig("test-typekit-gnulinux.pc", fields, res, true);
    BOOST_CHECK(st);

    BOOST_ASSERT(res.size() == expected_properties.size());
    for(size_t i=0; i<res.size(); i++){
        std::pair<std::string,std::string> expected = expected_properties[i];
        std::string val = res[i];
        BOOST_CHECK_EQUAL(val, expected.second);
    }
}
