#include "LoggingHelper.hpp"

#include <rtt/transports/corba/TaskContextProxy.hpp>
#include <rtt/OperationCaller.hpp>
#include <logger/proxies/Logger.hpp>
#include "CorbaNameService.hpp"
#include "Bundle.hpp"

LoggingHelper::LoggingHelper() : DEFAULT_LOG_BUFFER_SIZE(100)
{

}

bool LoggingHelper::logAllTask()
{
    CorbaNameService nameService;
    nameService.connect();
    
    nameService.getRegisteredTasks();
    
    //FAIL I can't iterate over all ports, because we are missing the typekits.
    
    return false;
    
}

bool LoggingHelper::logAllPorts(RTT::TaskContext* context, const std::string& loggerName, const std::vector< std::string > excludeList)
{
    std::string taskName = context->getName();
    
    std::cout << "Tryingt to get Proxy for " << loggerName << std::endl;

    logger::proxies::Logger *logger;
    
    try{
        logger = new logger::proxies::Logger(loggerName, false);
    } catch (...)
    {
        throw std::runtime_error("Error, could not contact the logger " + loggerName);
    }
    
    std::cout << "Managed to get Proxy for " << loggerName << std::endl;
    
    auto ports = context->ports()->getPorts();
    
    std::vector<RTT::base::OutputPortInterface *> outPorts;
    
    for(RTT::base::PortInterface *port: ports)
    {
        RTT::base::OutputPortInterface *outPort;
        outPort = dynamic_cast<RTT::base::OutputPortInterface *>(port);
        if(!outPort)
            continue;

        std::string name = taskName + "." + outPort->getName();

        if(std::find( excludeList.begin(), excludeList.end(), name) != excludeList.end())
        {
            std::cout << "logAllPorts: Excluding port " << name << " on task " << context->getName() << std::endl;
            continue;
        }
        

        //check if port allready exists
        RTT::base::PortInterface *loggerPort = logger->getPort(name);
        if(loggerPort)
        {
            RTT::base::InputPortInterface *inputPort = dynamic_cast<RTT::base::InputPortInterface *>(loggerPort);
            if(inputPort)
            {
                outPorts.push_back(outPort);
            }
        }
        
        
        std::cout << "Create Logging Port for " << name << std::endl;
        if(!logger->createLoggingPort(name, outPort->getTypeInfo()->getTypeName(), ::std::vector< ::logger::StreamMetadata >()))
        {
            std::cout << "logAllPorts: Error, failed to create port " << name << std::endl;
            return false;
        }
            
    }

    logger->synchronize();
    
    for(RTT::base::OutputPortInterface *outPort: outPorts)
    {
        //go save and disconnect everyone before doing the connection
        //we assume that we got exclusive control over the logger
        
        RTT::base::PortInterface *loggerPort = logger->getPort(taskName + "." + outPort->getName());
        if(!loggerPort)
        {
            throw std::runtime_error("Error, port created on logger could not be aquired"); 
        }
        loggerPort->disconnect();
        
        outPort->connectTo(loggerPort, RTT::ConnPolicy::buffer(DEFAULT_LOG_BUFFER_SIZE));
    }
 
    if(logger->isRunning())
        return true;

    Bundle &bundle(Bundle::getInstance());
    logger->file.set(bundle.getLogDirectory() + "/" + taskName + ".0.log"); 
    
    if(!logger->configure())
    {
        std::cout << "Failed to configure logger" << std::endl;
        return false;
    }

    if(!logger->start())
    {
        std::cout << "Failed to start logger for " + taskName << std::endl;
        return false;
    }
 
    return true;
}