#ifndef CORBANAMESERVICE_H
#define CORBANAMESERVICE_H

#include "NameService.hpp"
#include <omniORB4/CORBA.h>

namespace orocos_cpp
{

class CorbaNameService : public NameService
{
public:
    CorbaNameService(std::string name_service_ip="",std::string name_service_port = "");
    virtual ~CorbaNameService() {};
    virtual bool connect();
    virtual bool isConnected();
    virtual std::vector< std::string > getRegisteredTasks();
    virtual bool isRegistered(const std::string& taskName);
    virtual RTT::TaskContext* getTaskContext(const std::string& taskName);
    
private:
    bool initOrb();
    std::string ip;
    std::string port;
    CORBA::Object_var rootObj;
    CosNaming::NamingContext_var rootContext;
    CORBA::ORB_var orb;
};

}//end of namespace
#endif // CORBANAMESERVICE_H
