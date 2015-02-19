#include "CorbaNameService.hpp"
#include <rtt/transports/corba/ApplicationServer.hpp>
#include <rtt/transports/corba/TaskContextProxy.hpp>
#include <rtt/transports/corba/TaskContextC.h>
#include <stdexcept>
#include <iostream>

CorbaNameService::CorbaNameService(std::string name_service_ip, std::string name_service_port) : ip(name_service_ip), port(name_service_port)
{
}

bool CorbaNameService::connect()
{
    if(CORBA::is_nil(orb))
    {
        int argc = 0;
        char **argv = nullptr;
        if(!InitOrb(argc, argv))
        {
            throw std::runtime_error("CorbaNameService::Error, failed to initialize CORBA Orb");
        }
    }
    
    // NameService
    try {
        if(ip.empty())
        {
            rootObj = orb->resolve_initial_references("NameService");
        }
        else{
            std::string temp("corbaloc::");
            temp = temp + ip;
            if(!port.empty())
                temp = temp + ":" + port;
            temp = temp +"/NameService";
            rootObj = RTT::corba::ApplicationServer::orb->string_to_object(temp.c_str());
        }
    } catch (...) {}

    try {
        rootContext = CosNaming::NamingContext::_narrow(rootObj);
    } catch (...) {}

    if (CORBA::is_nil(rootContext)) {
        std::string err("TaskContextProxy could not acquire NameService.");
        std::cerr << err << std::endl;
        throw std::runtime_error(err);
    }
 
    return true;
}

std::vector< std::string > CorbaNameService::getRegisteredTasks()
{
    if(CORBA::is_nil(orb))
    {
        throw std::runtime_error("CorbaNameService::Error, called getRegisteredTasks() without connection " );
    }
    CosNaming::Name server_name;
    server_name.length(1);
    server_name[0].id = CORBA::string_dup("TaskContexts");
    std::vector<std::string> task_names;
    CosNaming::BindingList_var binding_list;
    CosNaming::BindingIterator_var binding_it;

    // get all available task names from the name server
    CORBA::Object_var control_tasks_var = rootContext->resolve(server_name);
    CosNaming::NamingContext_var control_tasks = CosNaming::NamingContext::_narrow (control_tasks_var);
    if (CORBA::is_nil(control_tasks))
        return task_names;

    control_tasks->list(0, binding_list, binding_it);
    if (CORBA::is_nil(binding_it))
        return task_names;

    // iterate over all task names
    while(binding_it->next_n(10, binding_list))
    {
        CosNaming::BindingList list = binding_list.in();
        for (unsigned int i = 0; i < list.length(); ++i)
        {
            std::string name = list[i].binding_name[0].id.in();
            CosNaming::Name serverName;
            serverName.length(2);
            serverName[0].id = CORBA::string_dup("TaskContexts");
            serverName[1].id = CORBA::string_dup( name.c_str() );

            try {
                //verify that the object really exists, and is not just some leftover from a crash
                
                // Get object reference
                CORBA::Object_var task_object = rootContext->resolve(serverName);
                RTT::corba::CTaskContext_var mtask = RTT::corba::CTaskContext::_narrow (task_object.in ());
                
                std::cout << "Connection test to " << name << std::endl;
                
                // force connect to object.
                CORBA::String_var nm = mtask->getName(); 
                task_names.push_back(name);
                }
            catch (...)
            {
            }
        }
    }
    return task_names;
}

bool CorbaNameService::isRegistered(const std::string& taskName)
{
    if(CORBA::is_nil(orb))
    {
        throw std::runtime_error("CorbaNameService::Error, called getTaskContext() without connection " );
    }

    CosNaming::Name serverName;
    serverName.length(2);
    serverName[0].id = CORBA::string_dup("TaskContexts");
    serverName[1].id = CORBA::string_dup( taskName.c_str() );

    try {
        // Get object reference
        CORBA::Object_var task_object = rootContext->resolve(serverName);
        if(CORBA::is_nil(task_object))
            return false;

        RTT::corba::CTaskContext_var mtask = RTT::corba::CTaskContext::_narrow (task_object.in ());
        if ( CORBA::is_nil( mtask ) ) {
            return false;
        }
        
        // force connect to object.
        //this needs to be done. If not, we may return a ghost task
        CORBA::String_var nm = mtask->getName(); 
    } catch (...)
    {
        return false;
    }
    

    
    return true;
}

RTT::TaskContext* CorbaNameService::getTaskContext(const std::string& taskName)
{
    if(CORBA::is_nil(orb))
    {
        throw std::runtime_error("CorbaNameService::Error, called getTaskContext() without connection " );
    }

    CosNaming::Name serverName;
    serverName.length(2);
    serverName[0].id = CORBA::string_dup("TaskContexts");
    serverName[1].id = CORBA::string_dup( taskName.c_str() );

    // Get object reference
    CORBA::Object_var task_object = rootContext->resolve(serverName);
    CORBA::String_var s = RTT::corba::ApplicationServer::orb->object_to_string(task_object);

    RTT::TaskContext *ret = nullptr;
        
    try
    {
        ret = RTT::corba::TaskContextProxy::Create(s.in(), true);;
    }
    catch (...)
    {
        std::cout << "Ghost " << taskName << std::endl;
    }
    
    return ret;
}
