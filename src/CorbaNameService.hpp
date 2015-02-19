#ifndef CORBANAMESERVICE_H
#define CORBANAMESERVICE_H

#include "NameService.hpp"
#include <rtt/transports/corba/ApplicationServer.hpp>

class CorbaNameService : public NameService, protected RTT::corba::ApplicationServer
{
public:
    CorbaNameService(std::string name_service_ip="",std::string name_service_port = "");
    virtual bool connect();
    virtual std::vector< std::string > getRegisteredTasks();
    virtual bool isRegistered(const std::string& taskName);
    virtual RTT::TaskContext* getTaskContext(const std::string& taskName);
    
private:
    std::string ip;
    std::string port;
    CORBA::Object_var rootObj;
    CosNaming::NamingContext_var rootContext;
};

#endif // CORBANAMESERVICE_H
