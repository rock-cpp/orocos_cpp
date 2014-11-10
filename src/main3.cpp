#include "ConfigurationHelper.hpp"
#include <rtt/TaskContext.hpp>
#include <rtt/transports/corba/TaskContextProxy.hpp>
#include <rtt/transports/corba/TaskContextServer.hpp>

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

int main(int argc, char**argv)
{
    loadAllPluginsInDir("/home/scotch/coyote/install/lib/orocos/gnulinux/types/");
    loadAllPluginsInDir("/home/scotch/coyote/install/lib/orocos/types/");

    RTT::corba::TaskContextServer::InitOrb(argc, argv);

    RTT::TaskContext *proxy = RTT::corba::TaskContextProxy::Create("orogen_default_tilt_scan__Task", false);
    if(!proxy)
    {
        std::cout << "Error, could not get task context" << std::endl;
        return 0;
    }

    ConfigurationHelper helper;
    helper.loadConfigFile("config.yml");
    
    std::vector<std::string> names;
    
    names.push_back("default");
    names.push_back("ground_based_sweeping");
//     names.push_back("foo");


    if(!helper.applyConfig(proxy, names))
        std::cout << "APPLY FAILED" << std::endl;
    
    return 0;
}