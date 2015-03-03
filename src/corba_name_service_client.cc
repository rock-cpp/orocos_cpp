/*
 Copyright (c) 2012, Alexander Duda, DFKI
 */

//Topic used to find registered Orocos Task Contexts
#define TOPIC "TaskContexts"    

#include "corba_name_service_client.hh"
#include <rtt/transports/corba/ApplicationServer.hpp>

using namespace corba;

NameServiceClient::NameServiceClient(std::string name_service_ip,std::string name_service_port):
    name_service_port(name_service_port),
    name_service_ip(name_service_ip),
    abort_flag(false)
{
}

NameServiceClient::~NameServiceClient()
{
}

void NameServiceClient::reset(std::string const &ip,std::string const &port)
{
    boost::mutex::scoped_lock lock(mut);
    root_context = CosNaming::NamingContext::_nil();
    name_service_ip = ip;
    name_service_port = port;
}

CosNaming::NamingContext_var NameServiceClient::getNameService()
{
    boost::mutex::scoped_lock lock(mut);
    //get name service
    if (CORBA::is_nil(root_context))
        root_context = NameServiceClient::getNameService(name_service_ip,name_service_port);
    return root_context;
}

std::string NameServiceClient::getIp()
{
    boost::mutex::scoped_lock lock(mut);
    return name_service_ip;
}

std::string NameServiceClient::getPort()
{
    boost::mutex::scoped_lock lock(mut);
    return name_service_port;
}

void NameServiceClient::abort()
{
    abort_flag = true;
}

std::vector<std::string> NameServiceClient::getTaskContextNames()
{
    // no need to lock mutex getNameService is taking care of this.
    abort_flag = false;
    CosNaming::Name server_name;
    server_name.length(1);
    server_name[0].id = CORBA::string_dup(TOPIC);
    std::vector<std::string> task_names;
    CosNaming::BindingList_var binding_list;
    CosNaming::BindingIterator_var binding_it;
    CosNaming::NamingContext_var root_context = getNameService();

    // get all available task names from the name server
    CORBA::Object_var control_tasks_var = root_context->resolve(server_name);
    CosNaming::NamingContext_var control_tasks = CosNaming::NamingContext::_narrow (control_tasks_var);
    if (CORBA::is_nil(control_tasks))
        return task_names;

    if(abort_flag)
        return task_names;

    control_tasks->list(0, binding_list, binding_it);
    if (CORBA::is_nil(binding_it))
        return task_names;

    // iterate over all task names
    while(!abort_flag && binding_it->next_n(10, binding_list))
    {
        CosNaming::BindingList list = binding_list.in();
        for (unsigned int i = 0; i < list.length(); ++i)
            task_names.push_back(std::string(list[i].binding_name[0].id.in()));
    }
    return task_names;
}

void NameServiceClient::bind(CORBA::Object_var const &obj,std::string const& name)
{
    // no need to lock mutex getNameService is taking care of this.
    CosNaming::NamingContext_var root_context = getNameService();
    CosNaming::Name n;
    n.length(1);
    n[0].id = CORBA::string_dup(TOPIC);
    try
    {
        CosNaming::NamingContext_var nc = root_context->bind_new_context(n);
    }
    catch (const CosNaming::NamingContext::AlreadyBound &) 
    {
        // Fine, context already exists.
    }
    // Force binding
    n.length(2);
    n[0].id = CORBA::string_dup(TOPIC);
    n[1].id = CORBA::string_dup(name.c_str());
    root_context->rebind(n, obj);
}

void NameServiceClient::validate()
{
    // no need to lock mutex getNameService is taking care of this.
    getNameService();
}

bool NameServiceClient::unbind(std::string const& name)
{
    // no need to lock mutex getNameService is taking care of this.
    CosNaming::NamingContext_var root_context = getNameService();
    CosNaming::Name server_name;
    try
    {
        server_name.length(2);
        server_name[0].id = CORBA::string_dup( TOPIC );
        server_name[1].id = CORBA::string_dup( name.c_str() );
        root_context->unbind(server_name);
        return true;
    }
    catch(CosNaming::NamingContext::NotFound) {}
    return false;
}

std::string NameServiceClient::getIOR(std::string const& name)
{
    // no need to lock mutex getNameService is taking care of this.
    CosNaming::NamingContext_var root_context = getNameService();
    CosNaming::Name server_name;
    server_name.length(2);
    server_name[0].id = CORBA::string_dup(TOPIC);
    server_name[1].id = CORBA::string_dup( name.c_str() );
    CORBA::Object_var task_object;
    task_object = root_context->resolve(server_name);
    CORBA::String_var s = RTT::corba::ApplicationServer::orb->object_to_string(task_object);
    return std::string(s.in());
}

//returns a valid context or throws an exception
CosNaming::NamingContext_var NameServiceClient::getNameService(const std::string name_service_ip, const std::string name_service_port)
{
    if(CORBA::is_nil(RTT::corba::ApplicationServer::orb))
        throw NameServiceClientError("Corba is not initialized. Call Orocos.initialize first.");

    CosNaming::NamingContext_var rootContext;

    // Obtain reference to Root POA.
    CORBA::Object_var obj_poa = RTT::corba::ApplicationServer::orb->resolve_initial_references("RootPOA");
    PortableServer::POA_var root_poa = PortableServer::POA::_narrow(obj_poa);
    if(CORBA::is_nil(root_poa))
        throw NameServiceClientError("Failed to narrow poa context.");

    // activate poa manager
    root_poa->the_POAManager()->activate();

    // Obtain reference to NameServiceClient
    CORBA::Object_var obj;
    if(!name_service_ip.empty())
    {
        std::string temp("corbaloc::");
        temp = temp + name_service_ip;
        if(!name_service_port.empty())
            temp = temp + ":" + name_service_port;
        temp = temp +"/NameService";
        obj = RTT::corba::ApplicationServer::orb->string_to_object(temp.c_str());
    }
    else
        obj = RTT::corba::ApplicationServer::orb->resolve_initial_references("NameService");

    rootContext = CosNaming::NamingContext::_narrow(obj.in());
    if(CORBA::is_nil(rootContext))
        throw NameServiceClientError("Failed to narrow NameService context.");

    return rootContext;
}
