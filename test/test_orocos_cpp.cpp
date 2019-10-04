#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE "test_orocos_cpp"
#define BOOST_AUTO_TEST_MAIN

#include "orocos_cpp/orocos_cpp.hpp"
#include <boost/bind.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/test/execution_monitor.hpp>
#include <stdlib.h>


using namespace orocos_cpp;
BOOST_AUTO_TEST_CASE(initialize)
{
    OrocosCpp rock;
    int ret = putenv("PKG_CONFIG_PATH=../../test/test_pkgconfig");
    OrocosCppConfig cfg;
    bool st = rock.initialize(cfg);
    BOOST_CHECK(st);
    std::vector<std::string> tkn = rock.package_registry->getRegisteredTypekitNames();

    cfg.load_all_packages = true;
    cfg.init_corba = false;
    st = rock.initialize(cfg);
    tkn = rock.package_registry->getRegisteredTypekitNames();
    BOOST_CHECK_EQUAL(tkn.size(), 1);
}

