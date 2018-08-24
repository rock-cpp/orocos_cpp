#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE "test_typeregistry"
#define BOOST_AUTO_TEST_MAIN

#include <boost/bind.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/test/execution_monitor.hpp>  

#include <orocos_cpp/TypeRegistry.hpp>

using namespace orocos_cpp;

BOOST_AUTO_TEST_CASE( unknown_names )
{
    orocos_cpp::TypeRegistry type_registries;
    type_registries.loadTypeRegistries();
    unsigned int id;
    std::string state_name = "CONTROLLING";
    std::string wrong_state_name = "UNKNOWNSTATENAME";
    BOOST_CHECK(!type_registries.getStateID("WrongTaskName::AccelerationController", state_name, id));
    BOOST_CHECK(!type_registries.getStateID("auv_control::WrongModelName", state_name, id));
    BOOST_CHECK(!type_registries.getStateID("auv_control::AccelerationController", wrong_state_name, id));
}

BOOST_AUTO_TEST_CASE( testfile )
{
    orocos_cpp::TypeRegistry type_registries;
    type_registries.loadTypeRegistries();
    unsigned int controlling_id;
    unsigned int exception_id;
    std::string controlling_state = "CONTROLLING";
    std::string exception_state = "EXCEPTION";
    BOOST_CHECK(type_registries.getStateID("auv_control::AccelerationController", controlling_state, controlling_id));
    BOOST_CHECK(type_registries.getStateID("auv_control::AccelerationController", exception_state, exception_id));
    BOOST_REQUIRE_EQUAL(controlling_id, 7);
    BOOST_REQUIRE_EQUAL(exception_id, 3);
}
