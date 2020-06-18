#include "LoggerProxy.hpp"

#include <rtt/typekit/RealTimeTypekit.hpp>
#include <rtt/transports/corba/TransportPlugin.hpp>
#include <rtt/transports/mqueue/TransportPlugin.hpp>

#include <rtt/types/TypekitPlugin.hpp>
    
#include "PluginHelper.hpp"

namespace orocos_cpp {

const std::string LoggerProxy::ModelName("logger::Logger");
    
LoggerProxyInitializer::LoggerProxyInitializer(std::string location, bool is_ior)
:TaskContextProxy()
{
    initTypes();
    
    try {
        initFromURIOrTaskname(location, is_ior);
    } catch (...)
    {
        throw std::runtime_error("Error : Failed to lookup task context " + location);
    }
    
    RTT::OperationInterfacePart *opIfac = getOperation("getModelName");
    RTT::OperationCaller< ::std::string() >  caller(opIfac);
    if(caller() != LoggerProxy::ModelName)
    {
        throw std::runtime_error("Error : Type mismatch in proxy-generation for " + location + 
                                 ": Proxy was of type '" + LoggerProxy::ModelName + 
                                 "' but task was of type '" + caller() + "'");
    }
}

void LoggerProxyInitializer::initTypes()
{
    //dynamically load the base and logger typekits
    if(!PluginHelper::loadTypekitAndTransports("std"))
    {
        std::cout << "LoggerHelper : Error, could not load std typekit, is it not installed ?" << std::endl;
        throw std::runtime_error("LoggerHelper : Error, could not load std typekit, is it not installed ?");
    }

    if(!PluginHelper::loadTypekitAndTransports("base"))
    {
        std::cout << "LoggerHelper : Error, could not load base typekit, is it not installed ?" << std::endl;
        throw std::runtime_error("LoggerHelper : Error, could not load base typekit, is it not installed ?");
    }
   
    if(!PluginHelper::loadTypekitAndTransports("logger"))
    {
        std::cout << "LoggerHelper : Error, could not load logger typekit, is it not installed ?" << std::endl;
        throw std::runtime_error("LoggerHelper : Error, could not load logger typekit, is it not installed ?");
    }
   
}
    
LoggerProxy::LoggerProxy(std::string location, bool is_ior) :
LoggerProxyInitializer(location, is_ior),
state(getPort("state")),
file(*(dynamic_cast<RTT::Property< ::std::string > *>(getProperty("file")))),
overwrite_existing_files(*(dynamic_cast<RTT::Property< bool > *>(getProperty("overwrite_existing_files"))))
{
}

void LoggerProxy::synchronize()
{
        RTT::corba::TaskContextProxy::synchronize();
}

bool LoggerProxy::reportComponent(::std::string const & name){
RTT::OperationInterfacePart *opIfac = getOperation("reportComponent");
RTT::OperationCaller< bool(::std::string const &) >  caller(opIfac);
return caller( name);
}
bool LoggerProxy::unreportComponent(::std::string const & name){
RTT::OperationInterfacePart *opIfac = getOperation("unreportComponent");
RTT::OperationCaller< bool(::std::string const &) >  caller(opIfac);
return caller( name);
}
bool LoggerProxy::reportPort(::std::string const & component, ::std::string const & port){
RTT::OperationInterfacePart *opIfac = getOperation("reportPort");
RTT::OperationCaller< bool(::std::string const &, ::std::string const &) >  caller(opIfac);
return caller( component,  port);
}
bool LoggerProxy::unreportPort(::std::string const & component, ::std::string const & port){
RTT::OperationInterfacePart *opIfac = getOperation("unreportPort");
RTT::OperationCaller< bool(::std::string const &, ::std::string const &) >  caller(opIfac);
return caller( component,  port);
}
bool LoggerProxy::createLoggingPort(const std::string& port_name, const std::string& type_name){
    RTT::OperationInterfacePart *opIfac = getOperation("createLoggingPort");

    RTT::internal::ConstantDataSource<std::string>::shared_ptr arg1(new RTT::internal::ConstantDataSource<std::string>(port_name));
    RTT::internal::ConstantDataSource<std::string>::shared_ptr arg2(new RTT::internal::ConstantDataSource<std::string>(type_name));
    
    const RTT::types::TypeInfo *metaData = opIfac->getArgumentType(3);
    RTT::base::DataSourceBase::shared_ptr arg3 = metaData->construct({});
    RTT::base::DataSourceBase::shared_ptr returnVal =  opIfac->produce({arg1, arg2, arg3}, nullptr);
    
    RTT::internal::DataSource<bool> *retValPtr = dynamic_cast<RTT::internal::DataSource<bool> *>(returnVal.get());
    return retValPtr->get();
}
bool LoggerProxy::removeLoggingPort(::std::string const & port_name){
RTT::OperationInterfacePart *opIfac = getOperation("removeLoggingPort");
RTT::OperationCaller< bool(::std::string const &) >  caller(opIfac);
return caller( port_name);
}
void LoggerProxy::clear(){
RTT::OperationInterfacePart *opIfac = getOperation("clear");
RTT::OperationCaller< void() >  caller(opIfac);
return caller();
}
::std::string LoggerProxy::getModelName(){
RTT::OperationInterfacePart *opIfac = getOperation("getModelName");
RTT::OperationCaller< ::std::string() >  caller(opIfac);
return caller();
}
boost::int32_t LoggerProxy::__orogen_getTID(){
RTT::OperationInterfacePart *opIfac = getOperation("__orogen_getTID");
RTT::OperationCaller< boost::int32_t() >  caller(opIfac);
return caller();
}
        
        
}
