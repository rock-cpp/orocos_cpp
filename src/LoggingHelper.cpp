#include "LoggingHelper.hpp"

#include <rtt/transports/corba/TaskContextProxy.hpp>
#include <rtt/OperationCaller.hpp>
#include <rtt/types/TypekitRepository.hpp>
#include <rtt/plugin/PluginLoader.hpp>
#include "LoggerProxy.hpp"
#include "CorbaNameService.hpp"
#include <lib_config/Bundle.hpp>
#include "Spawner.hpp"
#include "PluginHelper.hpp"
#include <boost/filesystem.hpp>

using namespace orocos_cpp;
using namespace libConfig;


LoggingHelper::LoggingHelper() : DEFAULT_LOG_BUFFER_SIZE(100)
{

}

bool LoggingHelper::logTasks()
{
    return logTasks(std::map<std::string, bool>(), true);
}


bool LoggingHelper::logTasks(const std::map<std::string, bool> &loggingEnabledTaskMap, bool logAll)
{
    Spawner &spawner(Spawner::getInstance());
    
    std::vector<const Deployment *> depls = spawner.getRunningDeployments();
    
    RTT::plugin::PluginLoader loader;

    if(!RTT::types::TypekitRepository::hasTypekit("rtt-types"))
        PluginHelper::loadTypekitAndTransports("rtt-types");
    
    for(const Deployment *dpl: depls)
    {
        //load all needed typekits
        for(const std::string &tk: dpl->getNeededTypekits())
        {
            if(!RTT::types::TypekitRepository::hasTypekit(tk))
            {
                
                std::cout << "Warning, we are missing the typekit " << tk << " loading it " << std::endl;
                PluginHelper::loadTypekitAndTransports(tk);
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
            
            if(logAll)
            {
                RTT::corba::TaskContextProxy *proxy = RTT::corba::TaskContextProxy::Create(task, false);
                logAllPorts(proxy, dpl->getLoggerName(),  std::vector< std::string >(), false);
                delete proxy;
            }
            else
            {
                auto it = loggingEnabledTaskMap.find(task);
                if(it != loggingEnabledTaskMap.end() && it->second)
                {
                    RTT::corba::TaskContextProxy *proxy = RTT::corba::TaskContextProxy::Create(task, false);
                    logAllPorts(proxy, dpl->getLoggerName(),  std::vector< std::string >(), false);
                    delete proxy;
                }
                else
                {
                    std::cout << "Logging for task " << task << " not enabled." << std::endl;
                }
            }
        }
    }
    
    return false;
}

bool LoggingHelper::logTasks(const std::vector< std::string >& excludeList)
{
    Spawner &spawner(Spawner::getInstance());
    
    std::vector<const Deployment *> depls = spawner.getRunningDeployments();
    
    RTT::plugin::PluginLoader loader;

    if(!RTT::types::TypekitRepository::hasTypekit("rtt-types"))
        PluginHelper::loadTypekitAndTransports("rtt-types");
    
    for(const Deployment *dpl: depls)
    {
        //load all needed typekits
        for(const std::string &tk: dpl->getNeededTypekits())
        {
            if(!RTT::types::TypekitRepository::hasTypekit(tk))
            {
                
                std::cout << "Warning, we are missing the typekit " << tk << " loading it " << std::endl;
                PluginHelper::loadTypekitAndTransports(tk);
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
            
            auto it = std::find(excludeList.begin(), excludeList.end(), task);
            if(it == excludeList.end())
            {
                RTT::corba::TaskContextProxy *proxy = RTT::corba::TaskContextProxy::Create(task, false);
                logAllPorts(proxy, dpl->getLoggerName(),  excludeList, false);
                delete proxy;
            }
            else
            {
                std::cout << "Logging for task " << task << " not enabled." << std::endl;
            }
        }
    }
    
    return false;
}



bool LoggingHelper::logAllPorts(RTT::TaskContext* givenContext, const std::string& loggerName, const std::vector< std::string > excludeList, bool loadTypekits)
{
    Bundle &bundle(Bundle::getInstance());
    return logAllPorts(givenContext, loggerName, bundle.getLogDirectory(), excludeList, loadTypekits);
}

bool LoggingHelper::logAllPorts(RTT::TaskContext* givenContext, const std::string& loggerName, const std::string& log_directory,
                                const std::vector< std::string > excludeList, bool loadTypekits)
{
    RTT::TaskContext* context = givenContext;
    std::string taskName = context->getName();
    
    std::cout << "Tryingt to get Proxy for " << loggerName << std::endl;

    if(loadTypekits)
    {
        PluginHelper::loadTypekitAndTransports("rtt-types");

        RTT::OperationCaller<std::string ()> getModelName(context->getOperation("getModelName"));
        std::string modelName = getModelName();
        std::string componentName = modelName.substr(0, modelName.find_first_of(':'));
        
        std::vector<std::string> neededTks = PluginHelper::getNeededTypekits(componentName);
        for(const std::string &tk: neededTks)
        {
            PluginHelper::loadTypekitAndTransports(tk);
        }
        
        //ugly, but only way I see to ensure that all ports get created
        context = RTT::corba::TaskContextProxy::Create(taskName, false);
    }
    
    LoggerProxy *logger;
    
    try{
        logger = new LoggerProxy(loggerName, false);
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
        if(!logger->createLoggingPort(name, outPort->getTypeInfo()->getTypeName()))
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
        
        RTT::ConnPolicy policy = RTT::ConnPolicy::buffer(DEFAULT_LOG_BUFFER_SIZE);
        policy.init = true;
        
        if(!outPort->connectTo(loggerPort, policy))
        {
            throw std::runtime_error("Error, could not connect port to logger");
        }
    }
 
    if(logger->isRunning())
        return true;

    int log_file_id = 0;
    std::string log_path(log_directory + "/" + loggerName + "." + std::to_string(log_file_id) + ".log");
    while(boost::filesystem::exists(log_path))
    {
        log_path = std::string(log_directory + "/" + loggerName + "." +  std::to_string(++log_file_id) + ".log");
    }
    logger->file.set(log_path);

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
 
    delete logger;
 
    return true;
}
