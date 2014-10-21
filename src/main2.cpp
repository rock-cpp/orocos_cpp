#include "ProxyPort.hpp"
#include <rtt/transports/corba/TaskContextProxy.hpp>
#include "corba.hh"
#include "corba_name_service_client.hh"
#include "OrocosHelpers.hpp"

class MirrorProxy: public RTT::corba::TaskContextProxy
{
public:
    MirrorProxy(std::string location, bool is_ior) : TaskContextProxy(location, is_ior), input(getPort("input")), output(getPort("output"))
    {
        
    };
    InputProxyPort<std::string> input;
    OutputProxyPort<std::string> output;
};

int main(int argc, char**argv)
{
    loadAllPluginsInDir("/home/scotch/coyote/install/lib/orocos/gnulinux/types/");
    
    loadAllPluginsInDir("/home/scotch/coyote/install/lib/orocos/types/");

    std::cout << "Plugin load done" << std::endl;
    MirrorProxy *mirrorProxy;
    
    CorbaAccess::init(argc, argv);
    CorbaAccess *acc = CorbaAccess::instance();
    
    std::cout << "Corba init done" << std::endl;
    
    corba::NameServiceClient client;
        
    std::vector<std::string> taskList;
    try {
         taskList = client.getTaskContextNames();
    } catch (CosNaming::NamingContext::NotFound e)
    {
        std::cout << "Could not get Task Context list." << std::endl;
        exit(EXIT_FAILURE);
    }
    
    for(std::vector<std::string>::const_iterator it = taskList.begin(); it != taskList.end(); it++)
    {
        if(*it == "orogen_default_mirror__Task")
        {
            std::string curIOR = client.getIOR(*it);
            
            mirrorProxy = new MirrorProxy(curIOR, true);            
        }
    }

    std::cout << "Configuring " << std::endl; 
    mirrorProxy->configure();
    std::cout << "Starting " << std::endl; 
    mirrorProxy->start();
    
    RTT::OutputPort<std::string> &writer(mirrorProxy->input.getWriter());
    RTT::InputPort<std::string> &reader(mirrorProxy->output.getReader());
    
    writer.write(std::string("Testooo"));
    std::cout << "Wrote sample " << std::endl; 
    
    std::string result;
    
    while(reader.read(result) != RTT::NewData)
    {
        usleep(10000);
    }
    
    std::cout << "result is " << result << std::endl;
    
    return 0;
}