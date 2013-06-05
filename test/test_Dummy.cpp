#include <boost/test/unit_test.hpp>
#include <dummy_project/Dummy.hpp>

using namespace dummy_project;

BOOST_AUTO_TEST_CASE(it_should_not_crash_when_welcome_is_called)
{
    dummy_project::DummyClass dummy;
    dummy.welcome();
}
