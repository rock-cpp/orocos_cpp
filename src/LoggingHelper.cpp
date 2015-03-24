#include "LoggingHelper.hpp"

#include <rtt/transports/corba/TaskContextProxy.hpp>
#include <rtt/OperationCaller.hpp>
#include <rtt/types/TypekitRepository.hpp>
#include <rtt/plugin/PluginLoader.hpp>
#include <logger/proxies/Logger.hpp>
#include "CorbaNameService.hpp"
#include "Bundle.hpp"
#include "Spawner.hpp"

LoggingHelper::LoggingHelper() : DEFAULT_LOG_BUFFER_SIZE(100)
{

}

bool LoggingHelper::logAllTasks()
{
    Spawner &spawner(Spawner::getInstace());
    
    std::vector<const Deployment *> depls = spawner.getRunningDeployments();
    
    RTT::plugin::PluginLoader loader;

    if(!RTT::types::TypekitRepository::hasTypekit("rtt-types"))
        OrocosHelpers::loadTypekitAndTransports("rtt-types");
    
    for(const Deployment *dpl: depls)
    {
        //load all needed typekits
        for(const std::string &tk: dpl->getNeededTypekits())
        {
            if(!RTT::types::TypekitRepository::hasTypekit(tk))
            {
                
                std::cout << "Warning, we are missing the typekit " << tk << " loading it " << std::endl;
                OrocosHelpers::loadTypekitAndTransports(tk);
            }

            if(!RTT::types::TypekitRepository::hasTypekit(tk))
            {
                std::cout << "Load failed" << std::endl;
            }
        }
        
        for(const std::string &task: dpl->getTaskNames())
        {
            //don't log the logger :-)
            if(task == dpl->getLoggerName())
                continue;
            
            RTT::corba::TaskContextProxy *proxy = RTT::corba::TaskContextProxy::Create(task, false);
            logAllPorts(proxy, dpl->getLoggerName(),  std::vector< std::string >(), false);
        }
    }
    
    return false;
}

bool LoggingHelper::logAllPorts(RTT::TaskContext* givenContext, const std::string& loggerName, const std::vector< std::string > excludeList, bool loadTypekits)
{
    RTT::TaskContext* context = givenContext;
    std::string taskName = context->getName();
    
    std::cout << "Tryingt to get Proxy for " << loggerName << std::endl;

    if(loadTypekits)
    {
        OrocosHelpers::loadTypekitAndTransports("rtt-types");

        RTT::OperationCaller<std::string ()> getModelName(context->getOperation("getModelName"));
        std::string modelName = getModelName();
        std::string componentName = modelName.substr(0, modelName.find_first_of(':'));
        
        std::vector<std::string> neededTks = OrocosHelpers::getNeededTypekits(componentName);
        for(const std::string &tk: neededTks)
        {
            OrocosHelpers::loadTypekitAndTransports(tk);
        }
        
        //ugly, but only way I see to ensure that all ports get created
        context = RTT::corba::TaskContextProxy::Create(taskName, false);
    }
    
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
            continue;
        }
        
        
        std::cout << "Create Logging Port for " << name << std::endl;
        if(!logger->createLoggingPort(name, outPort->getTypeInfo()->getTypeName(), ::std::vector< ::logger::StreamMetadata >()))
        {
            std::cout << "logAllPorts: Error, failed to create port " << name << std::endl;
            return false;
        }

        outPorts.push_back(outPort);
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
        
        if(!outPort->connectTo(loggerPort, RTT::ConnPolicy::buffer(DEFAULT_LOG_BUFFER_SIZE)))
        {
            throw std::runtime_error("Error, could not connect port to logger");
        }
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
 
    if(givenContext != context)
    {
        delete context;
    }
 
    return true;
}