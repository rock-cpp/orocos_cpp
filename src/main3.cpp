#include "ConfigurationHelper.hpp"
#include <rtt/transports/corba/TaskContextProxy.hpp>
#include <rtt/transports/corba/TaskContextServer.hpp>

#include "Spawner.hpp"
#include "TransformerHelper.hpp"
#include "PluginHelper.hpp"

int main(int argc, char**argv)
{
	using namespace orocoscpp;
    RTT::corba::TaskContextServer::InitOrb(argc, argv);

    PluginHelper::loadAllPluginsInDir("/home/scotch/coyote/install/lib/orocos/gnulinux/types/");
    PluginHelper::loadAllPluginsInDir("/home/scotch/coyote/install/lib/orocos/types/");

    Spawner &spawner(Spawner::getInstace());
    
    spawner.spawnTask("hokuyo::Task", "hokuyo");

    spawner.waitUntilAllReady(base::Time::fromSeconds(2.0));

//     std::cout << "All ready" << std::endl;
    
//     usleep(100000);

    RTT::TaskContext *proxy = RTT::corba::TaskContextProxy::Create("hokuyo", false);
    if(!proxy)
    {
        std::cout << "Error, could not get task context" << std::endl;
        return 0;
    }
    
//     std::cout << "Waited " << cnt << " iterations" << std::endl;
    
    ConfigurationHelper helper;

    smurf::Robot foo;
    TransformerHelper trHelper(foo);
    
    trHelper.configureTransformer(proxy);
    
    if(!helper.applyConfig(proxy, "default", "ground_based_sweeping"))
        std::cout << "APPLY FAILED" << std::endl;
    
    return 0;
}
