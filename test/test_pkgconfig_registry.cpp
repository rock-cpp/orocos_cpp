#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE "test_pkgconfig_helper"
#define BOOST_AUTO_TEST_MAIN

#include <boost/bind.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/test/execution_monitor.hpp>

#include <orocos_cpp/PkgConfigRegistry.hpp>
#include <stdlib.h>

using namespace orocos_cpp;
BOOST_AUTO_TEST_CASE(loadAllPackages)
{
    int ret = putenv("PKG_CONFIG_PATH=../../test/test_pkgconfig");
    PkgConfigRegistry reg({}, true);

    PkgConfig pkg;
    BOOST_CHECK(reg.getDeployment("ping_pong_aba_a", pkg));
    std::string val;
    BOOST_CHECK(pkg.getVariable("project_name", val));
    BOOST_CHECK_EQUAL(val, "rrt_evaluation_deployments");


    TypekitPkgConfig typ;
    BOOST_CHECK(reg.getTypekit("aggregator", typ));
    BOOST_ASSERT(typ.transports.size() == 3);
    pkg = typ.transports["corba"];
    BOOST_CHECK(pkg.getProperty("Name", val));
    BOOST_CHECK_EQUAL(val, "aggregatorCorbaTransport");
    pkg = typ.transports["mqueue"];
    BOOST_CHECK(pkg.getProperty("Name", val));
    BOOST_CHECK_EQUAL(val, "aggregatorMQueueTransport");
    pkg = typ.transports["typelib"];
    BOOST_CHECK(pkg.getProperty("Name", val));
    BOOST_CHECK_EQUAL(val, "aggregatorTypelibTransport");

    BOOST_CHECK(typ.typekit.getProperty("Name", val));
    BOOST_CHECK_EQUAL(val, "aggregatorTypekit");

    OrogenPkgConfig oro;
    BOOST_CHECK(reg.getOrogen("aggregator", oro));
    BOOST_CHECK(oro.project.getProperty("Description", val));
    BOOST_CHECK_EQUAL(val, "the definition file for the orogen project itself");
    BOOST_CHECK(!oro.proxies.isLoaded()); //Not present in folder
    BOOST_CHECK(!oro.tasks.isLoaded()); //Not present in folder

    BOOST_CHECK(reg.getOrogen("execution", oro));
    BOOST_CHECK(oro.project.isLoaded());
    BOOST_CHECK(oro.proxies.isLoaded());
    BOOST_CHECK(oro.tasks.isLoaded());

    std::vector<std::string> expected_orogen = {"aggregator", "execution"};
    std::vector<std::string> expected_deployments = {"ping_pong_aba_a"};
    std::vector<std::string> expected_typekits = {"aggregator"};

    std::vector<std::string> regoro = reg.getRegisteredOrogenNames();
    BOOST_CHECK(regoro == expected_orogen);
    std::vector<std::string> regdep = reg.getRegisteredDeploymentNames();
    BOOST_CHECK(regdep == expected_deployments);
    std::vector<std::string> regtyp = reg.getRegisteredTypekitNames();
    BOOST_CHECK(regtyp == expected_typekits);
}

BOOST_AUTO_TEST_CASE(loadNamedPackages)
{
    int ret = putenv("PKG_CONFIG_PATH=../../test/test_pkgconfig");
    PkgConfigRegistry reg({"ping_pong_aba_a"}, false);

    PkgConfig pkg;
    //Only the named package is loaded
    BOOST_CHECK_EQUAL(reg.getRegisteredDeploymentNames().size(), 1);
    BOOST_CHECK_EQUAL(reg.getRegisteredOrogenNames().size(), 0);
    BOOST_CHECK_EQUAL(reg.getRegisteredTypekitNames().size(), 0);
    std::vector<std::string> packages = reg.getRegisteredDeploymentNames();
    BOOST_CHECK(std::find(packages.begin(),packages.end(),"ping_pong_aba_a") != packages.end());

    //Not-pre-loaded packages can still be searched and post-loaded
    TypekitPkgConfig typ;
    BOOST_CHECK(reg.getTypekit("aggregator", typ));
    BOOST_CHECK_EQUAL(reg.getRegisteredDeploymentNames().size(), 1);
    BOOST_CHECK_EQUAL(reg.getRegisteredOrogenNames().size(), 1);
    BOOST_CHECK_EQUAL(reg.getRegisteredTypekitNames().size(), 1);

    OrogenPkgConfig oro;
    BOOST_CHECK(reg.getOrogen("execution", oro));
    BOOST_CHECK_EQUAL(reg.getRegisteredDeploymentNames().size(), 1);
    BOOST_CHECK_EQUAL(reg.getRegisteredOrogenNames().size(), 2);
    BOOST_CHECK_EQUAL(reg.getRegisteredTypekitNames().size(), 1);

    BOOST_CHECK_EQUAL(reg.getOrocosRTT(pkg, false), false);
    BOOST_CHECK_EQUAL(reg.getOrocosRTT(pkg, true), true);

    //Terminates if unknown apcakge is requested
    BOOST_CHECK(reg.getOrogen("gibt'snicht", oro) == false);
}

