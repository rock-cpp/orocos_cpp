#include "corba.hh"
#include "corba_name_service_client.hh"

#include <list>
#include <typeinfo>
#include <rtt/types/Types.hpp>

#include <rtt/transports/corba/TransportPlugin.hpp>
#include <rtt/transports/corba/CorbaLib.hpp>
#include <rtt/transports/corba/TaskContextServer.hpp>
#include <rtt/transports/corba/TaskContextProxy.hpp>
#include <rtt/transports/corba/CorbaDispatcher.hpp>

#include <rtt/Activity.hpp>
#include <boost/lexical_cast.hpp>

using namespace CORBA;
using namespace std;
using namespace boost;
using namespace corba;

CorbaAccess* CorbaAccess::the_instance = NULL;
void CorbaAccess::init(int argc, char* argv[])
{
    if (the_instance)
        return;
    the_instance = new CorbaAccess(argc, argv);
}
void CorbaAccess::deinit()
{
    delete the_instance;
    the_instance = NULL;
}

CorbaAccess::CorbaAccess(int argc, char* argv[])
{
    // First initialize the ORB. We use TaskContextProxy::InitORB as we will
    // have to create a servant for our local DataFlowInterface object.
    RTT::corba::TaskContextServer::InitOrb(argc, argv);
}

CorbaAccess::~CorbaAccess()
{
    RTT::corba::TaskContextServer::ShutdownOrb(true);
}

RTaskContext* CorbaAccess::createRTaskContext(std::string const& ior)
{
    std::auto_ptr<RTaskContext> new_context( new RTaskContext );
    // check if ior is a valid IOR if not an exception is thrown
    new_context->task = getCTaskContext(ior);
    new_context->main_service = new_context->task->getProvider("this");
    new_context->ports      = new_context->task->ports();
    CORBA::String_var nm =  new_context->task->getName();
    new_context->name = std::string(nm.in());
    return new_context.release();
}

RTT::corba::CTaskContext_var CorbaAccess::getCTaskContext(std::string const& ior)
{
    if(CORBA::is_nil(RTT::corba::ApplicationServer::orb))
        throw std::runtime_error("Corba is not initialized. Call Orocos.initialize first.");

    // Use the ior to create the task object reference,
    CORBA::Object_var task_object;
    try
    {
        task_object = RTT::corba::ApplicationServer::orb->string_to_object ( ior.c_str() );
    }
    catch(CORBA::SystemException &e)
    {
        throw InvalidIORError("given IOR " + ior + " is not valid");
    }

    // Then check we can actually access it
    RTT::corba::CTaskContext_var mtask;
    // Now downcast the object reference to the appropriate type
    mtask = RTT::corba::CTaskContext::_narrow (task_object.in());

    if(CORBA::is_nil( mtask ))
        throw std::runtime_error("cannot narrorw task context.");
    return mtask;
}

void CorbaAccess::setCallTimeout(int ms)
{
    omniORB::setClientCallTimeout(ms);
}

void CorbaAccess::setConnectTimeout(int ms)
{
    omniORB::setClientConnectTimeout(ms);
}



/* call-seq:
 *   Orocos::CORBA.transportable_type_names => name_list
 *
 * Returns an array of string that are the type names which can be transported
 * over the CORBA layer
 */

vector< string > CorbaAccess::getTransportableTypeNames()
{
    RTT::types::TypeInfoRepository::shared_ptr rtt_types =
        RTT::types::TypeInfoRepository::Instance();

    std::vector<std::string> result;
        
    vector<string> all_types = rtt_types->getTypes();
    for (vector<string>::iterator it = all_types.begin(); it != all_types.end(); ++it)
    {
        RTT::types::TypeInfo* ti = rtt_types->type(*it);
        vector<int> transports = ti->getTransportNames();
        if (find(transports.begin(), transports.end(), ORO_CORBA_PROTOCOL_ID) != transports.end())
        {
            result.push_back(*it);
        }
    }
    return result;
}

/*
static VALUE name_service_task_context_names(VALUE self)
{
    corba_must_be_initialized();

    NameServiceClient& name_service = get_wrapped<NameServiceClient>(self);
    std::vector<std::string> names;

    names = corba_blocking_fct_call_with_result(boost::bind(&NameServiceClient::getTaskContextNames,&name_service),
                              boost::bind(&NameServiceClient::abort,&name_service));

    VALUE result = rb_ary_new();
    for (vector<string>::const_iterator it = names.begin(); it != names.end(); ++it)
        rb_ary_push(result, rb_str_new2(it->c_str()));
    return result;
}


static VALUE name_service_unbind(VALUE self,VALUE task_name)
{
    corba_must_be_initialized();

    std::string name = StringValueCStr(task_name);
    NameServiceClient& name_service = get_wrapped<NameServiceClient>(self);
    bool result = corba_blocking_fct_call_with_result(boost::bind(&NameServiceClient::unbind,&name_service,name));
    return result ? Qtrue : Qfalse;
}

static VALUE name_service_validate(VALUE self)
{
    corba_must_be_initialized();

    NameServiceClient& name_service = get_wrapped<NameServiceClient>(self);
    corba_blocking_fct_call(boost::bind(&NameServiceClient::validate,&name_service));
    return Qnil;
}

static VALUE name_service_bind(VALUE self,VALUE task,VALUE task_name)
{
    corba_must_be_initialized();
    
    std::string name = StringValueCStr(task_name);
    NameServiceClient& name_service = get_wrapped<NameServiceClient>(self);
    RTaskContext& context = get_wrapped<RTaskContext>(task);
    CORBA::Object_var obj = CORBA::Object::_duplicate(context.task);
    corba_blocking_fct_call(boost::bind(&NameServiceClient::bind,&name_service,obj,name));
    return Qnil;
}

static VALUE name_service_ior(VALUE self,VALUE task_name)
{
    corba_must_be_initialized();

    std::string name = StringValueCStr(task_name);
    NameServiceClient& name_service = get_wrapped<NameServiceClient>(self);
    std::string ior = corba_blocking_fct_call_with_result(boost::bind(&NameServiceClient::getIOR,&name_service,name));
    return rb_str_new2(ior.c_str());
}*/


