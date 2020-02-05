#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE "test_configuration_helper"
#define BOOST_AUTO_TEST_MAIN

#include <boost/bind.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/test/execution_monitor.hpp>

#include "ConfigurationHelper.hpp"
#include "PkgConfigHelper.hpp"
#include "PluginHelper.hpp"
#include "PkgConfigRegistry.hpp"
#include "TypeRegistry.hpp"
#include <lib_config/YAMLConfiguration.hpp>
#include <lib_config/Configuration.hpp>
#include <typelib/csvoutput.hh>
#include <typelib/registry.hh>
#include <rtt/types/Types.hpp>
#include <base/samples/RigidBodyState.hpp>
#include <base/m_types/base_samples_RigidBodyState.hpp>
#include <rtt/typelib/TypelibMarshaller.hpp>
#include <rtt/typelib/TypelibMarshaller.hpp>
#include <base/typekit/Types.hpp>



using namespace orocos_cpp;

std::string get_rbs_yaml(){
    std::string ret = R"V0G0N(---
sourceFrame: laser
targetFrame: body
position: {data: [0.0, 0.0, 0.641305]}
orientation: {re: 1, im: [0,0,0]}
)V0G0N";
    return ret;
}


BOOST_AUTO_TEST_CASE(test_YAMLization)
{
    //Initialize Registgries
    //orocos_cpp::PkgConfigRegistryPtr package_registry = PkgConfigRegistry::initialize("base", false);
    orocos_cpp::TypeRegistry registry;
    bool st = registry.loadTypeRegistry("base");

    //Load Sample from YAML
    YAML::Node docs = YAML::Load(get_rbs_yaml());
    libConfig::YAMLConfigParser parser;
    std::shared_ptr<libConfig::ConfigValue> conf =  parser.getConfigValue(docs);

    //Convert to Tipelib value
    const Typelib::Type *type = registry.getTypeModel("/base/samples/RigidBodyState_m");

    //Assign ConfigValue to Typelib Value (and thereby write data to rbs)
    base::samples::RigidBodyState_m rbs;
    Typelib::Value value((void*)&rbs, *type);
    ConfigurationHelper helper;
    helper.applyConfOnTyplibValue(value, *conf);

    //Export back to YAML
    std::string ys = helper.getYamlString(value);

    //Manually parse yaml string..
    YAML::Node node = YAML::Load(ys);
    base::samples::RigidBodyState_m rbs2;
    rbs2.sourceFrame = node["sourceFrame"].as<std::string>();
    rbs2.targetFrame = node["targetFrame"].as<std::string>();
    rbs2.position.data[0] = node["position"]["data"][0].as<double>();
    rbs2.position.data[1] = node["position"]["data"][1].as<double>();
    rbs2.position.data[2] = node["position"]["data"][2].as<double>();

    BOOST_CHECK_EQUAL(rbs.sourceFrame, rbs2.sourceFrame);
    BOOST_CHECK_EQUAL(rbs.targetFrame, rbs2.targetFrame);
    BOOST_CHECK_EQUAL(rbs.position.data[0], rbs2.position.data[0]);
    BOOST_CHECK_EQUAL(rbs.position.data[1], rbs2.position.data[1]);
    BOOST_CHECK_EQUAL(rbs.position.data[2], rbs2.position.data[2]);


    //Try convienence function
    base::samples::RigidBodyState_m rbs3;
    helper.loadTypeFromYaml(rbs3, ys, *type);

    BOOST_CHECK_EQUAL(rbs.sourceFrame, rbs3.sourceFrame);
    BOOST_CHECK_EQUAL(rbs.targetFrame, rbs3.targetFrame);
    BOOST_CHECK_EQUAL(rbs.position.data[0], rbs3.position.data[0]);
    BOOST_CHECK_EQUAL(rbs.position.data[1], rbs3.position.data[1]);
    BOOST_CHECK_EQUAL(rbs.position.data[2], rbs3.position.data[2]);

    std::cout << ys <<std::endl;
    return;
}
