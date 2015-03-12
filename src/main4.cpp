#include "CorbaNameService.hpp"
#include <rtt/TaskContext.hpp>
#include <orocos_cpp_base/OrocosHelpers.hpp>
#include <rtt/internal/ConnectionManager.hpp>
#include "LoggingHelper.hpp"
#include "Bundle.hpp"

int main(int argc, char **argv)
{
    Bundle &bundle(Bundle::getInstance());

    loadAllPluginsInDir("/home/scotch/coyote/install/lib/orocos/gnulinux/types/");
    loadAllPluginsInDir("/home/scotch/coyote/install/lib/orocos/types/");
    
    CorbaNameService ns;
    ns.connect();
    
    std::vector<std::string> tasks = ns.getRegisteredTasks();
    
    LoggingHelper lh;
    
    for(std::string &taksName: tasks)
    {
        std::cout << "Got TaskContext " << taksName << std::endl;
        
        RTT::TaskContext *context = ns.getTaskContext(taksName);
//         lh.logAllPorts(context);
        
//         RTT::TaskContext *context = ns.getTaskContext(taksName);
//         if(context)
//         {
//             std::cout << "Got TaskContext " << taksName << " component says its name is " << context->getName() << std::endl;
//          
//             RTT::DataFlowInterface *dfi = context->ports();
//             std::vector<RTT::base::PortInterface *> ports = dfi->getPorts();
//             
//             for(RTT::base::PortInterface *port: ports)
//             {
//                 std::cout << "Port " << port->getName() << std::endl;
//                 const RTT::internal::ConnectionManager *conManager = port->getManager();
//                 auto channels = conManager->getChannels();
//                 for(RTT::internal::ConnectionManager::ChannelDescriptor desc: channels)
//                 {
//                     boost::shared_ptr<RTT::internal::ConnID> connID(desc.head);
//                     RTT::ConnPolicy policy = desc.get<2>();
//                     std::cout << "    Connection from " << connID->getInputTaskName() << "." << connID->getInputPortName() << " to " 
//                         << connID->getOutputTaskName() << "." << connID->getOutputPortName() << " of type ";
//                     
//                     switch(policy.type)
//                     {
//                         case RTT::ConnPolicy::BUFFER:
//                             std::cout << "Buffer";
//                             break;
//                         case RTT::ConnPolicy::DATA:
//                             std::cout << "Data";
//                             break;
//                         case RTT::ConnPolicy::CIRCULAR_BUFFER:
//                             std::cout << "Circular Buffer";
//                             break;
//                         case RTT::ConnPolicy::UNBUFFERED:
//                             std::cout << "Unbuffered";
//                             break;
//                     }
//                     
//                     std::cout << std::endl;
//                 }
//             }
//             
//         }
    }
    
    
    return 0;
}