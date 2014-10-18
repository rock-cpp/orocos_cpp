#include <iostream>
#include "corba.hh"
#include "corba_name_service_client.hh"
#include <rtt/transports/corba/TaskContextProxy.hpp>


#include <rtt/typekit/RealTimeTypekit.hpp>
#include <rtt/transports/corba/TransportPlugin.hpp>
#include <rtt/transports/mqueue/TransportPlugin.hpp>

#include <rtt/types/TypekitPlugin.hpp>
    
#include <boost/filesystem.hpp>
#include <rtt/plugin/PluginLoader.hpp>

void loadAllPluginsInDir(const std::string &path)
{
    boost::filesystem::path pluginDir(path);

    boost::shared_ptr<RTT::plugin::PluginLoader> loader = RTT::plugin::PluginLoader::Instance();
    
    boost::filesystem::directory_iterator end_it; // default construction yields past-the-end
    for (boost::filesystem::directory_iterator it( pluginDir );
        it != end_it; it++ )
    {
        if(boost::filesystem::is_regular_file(*it))
        {
            std::cout << "Found library " << *it << std::endl;
            loader->loadLibrary(it->path().string());
        }
    }

}

int main(int argc, char** argv)
{
//     RTT::types::TypekitRepository::Import( new RTT::types::RealTimeTypekitPlugin );
//     RTT::types::TypekitRepository::Import( new RTT::corba::CorbaLibPlugin );
//     RTT::types::TypekitRepository::Import( new RTT::mqueue::MQLibPlugin );

//     RTT::types::TypekitRepository::Import(new RTT::types::RealTimeTypekitPlugin);
//     RTT::types::TypekitRepository::Import(new RTT::corba::CorbaLibPlugin);
//     RTT::types::TypekitRepository::Import(new RTT::mqueue::MQLibPlugin);
    
    loadAllPluginsInDir("/home/scotch/coyote/install/lib/orocos/gnulinux/types/");
    
    loadAllPluginsInDir("/home/scotch/coyote/install/lib/orocos/types/");
//     RTT::plugin::PluginLoader::Instance()->loadLibrary();
    
//     RTT::types::TypekitRepository::Import( new orogen_typekits::proxytestTypekitPlugin );
//     RTT::types::TypekitRepository::Import( new orogen_typekits::proxytestCorbaTransportPlugin );
//     RTT::types::TypekitRepository::Import( new orogen_typekits::proxytestMQueueTransportPlugin );
//     RTT::types::TypekitRepository::Import( new orogen_typekits::proxytestTypelibTransportPlugin );
//     
//     RTT::types::TypekitRepository::Import( new orogen_typekits::baseTypekitPlugin );
//     RTT::types::TypekitRepository::Import( new orogen_typekits::baseCorbaTransportPlugin );
//     RTT::types::TypekitRepository::Import( new orogen_typekits::baseMQueueTransportPlugin );
//     RTT::types::TypekitRepository::Import( new orogen_typekits::baseTypelibTransportPlugin );
//     
//     RTT::types::TypekitRepository::Import( new orogen_typekits::stdTypekitPlugin );
//     RTT::types::TypekitRepository::Import( new orogen_typekits::stdCorbaTransportPlugin );
//     RTT::types::TypekitRepository::Import( new orogen_typekits::stdMQueueTransportPlugin );
//     RTT::types::TypekitRepository::Import( new orogen_typekits::stdTypelibTransportPlugin );
    
    
    CorbaAccess::init(argc, argv);
    CorbaAccess *acc = CorbaAccess::instance();

    corba::NameServiceClient client;
    
    std::vector<std::string> taskList = client.getTaskContextNames();
    
    RTT::base::PortInterface *portReader;
    
    for(std::vector<std::string>::const_iterator it = taskList.begin(); it != taskList.end(); it++)
    {
        std::cout << "Task : " << *it << std::endl;
        
        std::string curIOR = client.getIOR(*it);
//         std::cout << "IOR is " << curIOR  << std::endl;
        
        if(*it == "command_server")
        {
        
            RTT::corba::TaskContextProxy *proxy = RTT::corba::TaskContextProxy::Create(curIOR, true);
            RTT::DataFlowInterface *flowInterface = proxy->ports();
            std::vector<RTT::base::PortInterface*> ports = flowInterface->getPorts();
            std::cout << "Number ports is " << flowInterface->getPorts().size() << std::endl;
            for(std::vector<RTT::base::PortInterface*>::iterator it = ports.begin(); it != ports.end() ; it++)
            {
                RTT::base::PortInterface *iface = *it;
            
//                 portReader = iface->antiClone();
                
                RTT::base::InputPortInterface *input = dynamic_cast<RTT::base::InputPortInterface *>(iface);
                RTT::base::OutputPortInterface *output = dynamic_cast<RTT::base::OutputPortInterface *>(iface);
                
                std::cout << "Port name " << (iface)->getName() << " is input " << (input != 0)<<  " is  output"  << (output != 0) << std::endl;
            }
            RTT::TaskContext::PeerList peers = proxy->getPeerList();
            
            std::cout << "Number of peers " << peers.size() << std::endl;
            
            for(RTT::TaskContext::PeerList::const_iterator it = peers.begin(); it != peers.end(); it++)
            {
                std::cout << "Peers is " << *it << std::endl;
            }
        }
    }

//     while(true)
//     {
//         portReader;
//         usleep(10000);
//     }
    
    
    return 0;
}
