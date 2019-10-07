#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE "test_typeregistry"
#define BOOST_AUTO_TEST_MAIN

#include <boost/bind.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/test/execution_monitor.hpp>  

#include <orocos_cpp/TypeRegistry.hpp>

using namespace orocos_cpp;


class TypeRegistryTest: public TypeRegistry
{
public:
    TypeRegistryTest(): TypeRegistry(PkgConfigRegistryPtr(new PkgConfigRegistry({}, false))){}

    void loadStatesFromCustomPath(const std::string &path)
    {
        loadStateToIDMapping(path);
    }

    void loadTypesFromCustomPath(const std::string &path, const std::string &typekitName)
    {
        loadTypeToTypekitMapping(path, typekitName);
    }
};

struct Fixture
{
    Fixture()
    {
        type_registries.loadStatesFromCustomPath("testfile.tlb");
        type_registries.loadTypesFromCustomPath("testfile.typelist", "auv_control");
    }

    TypeRegistryTest type_registries;
};


BOOST_FIXTURE_TEST_CASE(test_typeregistry_taskStates_unknownNames, Fixture)
{
    unsigned int id;
    std::string state_name = "CONTROLLING";
    std::string wrong_state_name = "UNKNOWNSTATENAME";
    BOOST_CHECK(!type_registries.getStateID("WrongTaskName::AccelerationController", state_name, id));
    BOOST_CHECK(!type_registries.getStateID("auv_control::WrongModelName", state_name, id));
    BOOST_CHECK(!type_registries.getStateID("auv_control::AccelerationController", wrong_state_name, id));
}

BOOST_FIXTURE_TEST_CASE(test_typeregistry_taskStates, Fixture)
{
    unsigned int controlling_id;
    unsigned int exception_id;
    std::string controlling_state = "CONTROLLING";
    std::string exception_state = "EXCEPTION";
    BOOST_CHECK(type_registries.getStateID("auv_control::AccelerationController", controlling_state, controlling_id));
    BOOST_CHECK(type_registries.getStateID("auv_control::AccelerationController", exception_state, exception_id));
    BOOST_REQUIRE_EQUAL(controlling_id, 7);
    BOOST_REQUIRE_EQUAL(exception_id, 3);
}

BOOST_FIXTURE_TEST_CASE(test_typeregistry_typeToTypekit, Fixture)
{
    std::string typekitName;
    BOOST_CHECK(type_registries.getTypekitDefiningType("/auv_control/PIDState", typekitName));
    BOOST_CHECK_EQUAL(typekitName, "auv_control");
}
