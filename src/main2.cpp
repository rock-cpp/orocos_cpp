#include <orocos_cpp_base/ProxyPort.hpp>
#include <rtt/transports/corba/TaskContextProxy.hpp>
#include <rtt/Property.hpp>
#include "PluginHelper.hpp"

class MirrorProxy: public RTT::corba::TaskContextProxy
{
public:
    MirrorProxy(std::string location, bool is_ior) : TaskContextProxy(location, is_ior)
    , input(getPort("input"))
    , output(getPort("output"))
    , testProp(*dynamic_cast<RTT::Property<std::string> *>(getProperty("testProp")))
    {
        
    };
    InputProxyPort<std::string> input;
    OutputProxyPort<std::string> output;
    
    RTT::Property<std::string> &testProp;
};

int main(int argc, char**argv)
{
    orocoscpp::PluginHelper::loadAllPluginsInDir("/home/scotch/coyote/install/lib/orocos/gnulinux/types/");
    
    orocoscpp::PluginHelper::loadAllPluginsInDir("/home/scotch/coyote/install/lib/orocos/types/");

//     std::cout << "Plugin load done" << std::endl;
//     MirrorProxy *mirrorProxy;
//     
//     CorbaAccess::init(argc, argv);
//     
//     std::cout << "Corba init done" << std::endl;
//     
//     corba::NameServiceClient client;
//         
//     std::vector<std::string> taskList;
//     try {
//          taskList = client.getTaskContextNames();
//     } catch (CosNaming::NamingContext::NotFound e)
//     {
//         std::cout << "Could not get Task Context list." << std::endl;
//         exit(EXIT_FAILURE);
//     }
//     
//     for(std::vector<std::string>::const_iterator it = taskList.begin(); it != taskList.end(); it++)
//     {
//         if(*it == "orogen_default_mirror__Task")
//         {
//             std::string curIOR = client.getIOR(*it);
//             
//             mirrorProxy = new MirrorProxy(curIOR, true);            
//         }
//     }
// 
//     std::cout << "Configuring " << std::endl; 
//     mirrorProxy->configure();
//     std::cout << "Starting " << std::endl; 
//     mirrorProxy->start();
//     
//     RTT::OutputPort<std::string> &writer(mirrorProxy->input.getWriter());
//     RTT::InputPort<std::string> &reader(mirrorProxy->output.getReader());
//     
//     writer.write(std::string("Testooo"));
//     std::cout << "Wrote sample " << std::endl; 
//     
//     std::string result;
//     
//     while(reader.read(result) != RTT::NewData)
//     {
//         usleep(10000);
//     }
//     
//     
//     
//     std::cout << "result is " << result << std::endl;
//     
//     std::cout << "Property is " << mirrorProxy->testProp.get() << std::endl;
    
    return 0;
}
