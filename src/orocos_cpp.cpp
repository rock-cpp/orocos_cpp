#include "orocos_cpp.hpp"
#include <orocos_cpp/CorbaNameService.hpp>
#include <rtt/transports/corba/TaskContextServer.hpp>
#include <lib_config/YAMLConfiguration.hpp>
#include "PluginHelper.hpp"


namespace orocos_cpp {

//Taken from /media/wirkus/Data/development/rock-runtime/tools/orocos.rb/ext/rorocos/corba_name_service_client.cc
CosNaming::NamingContext_var getNameService(const std::string name_service_ip, const std::string name_service_port)
{
    if(CORBA::is_nil(RTT::corba::ApplicationServer::orb))
        throw std::runtime_error("Corba is not initialized. Call Orocos.initialize first.");

    CosNaming::NamingContext_var rootContext;

    // Obtain reference to Root POA.
    CORBA::Object_var obj_poa = RTT::corba::ApplicationServer::orb->resolve_initial_references("RootPOA");
    PortableServer::POA_var root_poa = PortableServer::POA::_narrow(obj_poa);
    if(CORBA::is_nil(root_poa))
        throw std::runtime_error("Failed to narrow poa context.");

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


    try{
        rootContext = CosNaming::NamingContext::_narrow(obj.in());
    }catch(CORBA::TRANSIENT ex){
        std::cerr << "CORBA::TRANSIENT received while trying to obtain reference to NameService Client" << std::endl;
    }
    if(CORBA::is_nil(rootContext)){
        std::cerr << "Failed to narrow NameService context." << std::endl;
        return nullptr;
    }

    return rootContext;
}

bool validateNameServiceClient(std::string hostname, std::string port=""){
    return getNameService(hostname, port);
}

bool set_corba_ns_host(std::string hostname_or_ip){
    char* envi = strdup(("ORBInitRef=NameService=corbaname::"+hostname_or_ip).c_str());
    int ret = putenv(envi);
    return ret;
}

bool setMaxMessageSize(size_t bytes){
    std::string env_str = "ORBgiopMaxMsgSize="+std::to_string(bytes);
    char *cstr = &(env_str[0]);
    if( putenv(cstr) !=0 )
    {
        fprintf(stderr,"putenv failed\n");
        return false;
    }
    return true;
}

bool initializeCORBA(int argc, char**argv, std::string host="")
{
    //Set set CORBA nameserver
    if(!host.empty()){
        set_corba_ns_host(host);
    }

    //Set CORBA max message size only if it was not set by the user
    if(getenv("ORBgiopMaxMsgSize")) {
        //It's already set by the user. Don't do anything
    } else {
        //Set to 100MB
        bool st = setMaxMessageSize(100*1000*1000);
    }

    //Do the initialization
    bool orb_st = RTT::corba::ApplicationServer::InitOrb(argc, argv);
    if(!orb_st){

        std::cerr << "\nError initializing CORBA application server" <<std::endl;
        return false;
    }
    if(!validateNameServiceClient(host, "")){
        std::cerr << "\nCould not connect to name service '"<<host<<"'" <<std::endl;
        return false;
    }
    return true;
}

bool OrocosCpp::initialize(const OrocosCppConfig& config, bool quiet)
{
    bool st;

    //Init CORBA
    if(config.init_corba){
        if(!quiet) std::cout << "Initializing CORBA.. " << std::endl;
        st = initializeCORBA(0, {}, config.corba_host);
        if(!st){
            std::cerr << "Failed to initialze CORBA" << std::endl;
            return false;
        }
    }

    //Init PkgConfig Registry
    if(!quiet) std::cout << "\nLoading Rock-packages.." << std::endl;
    package_registry = PkgConfigRegistry::initialize(config.package_initialization_whitelist, config.load_all_packages);
    if(!package_registry){
        std::cerr << "Error initializing Rock-packages" <<std::endl;
        return false;
    }

    //Load Typekits
    if(config.load_typekits){
        if(!quiet) std::cout << "\nLoading Typekits.." << std::endl;
        for(std::string tkn : package_registry->getRegisteredTypekitNames())
        {
            st = PluginHelper::loadTypekitAndTransports(tkn);
        }
    }


    //Init Type Registry
    type_registry = TypeRegistryPtr(new TypeRegistry(package_registry));
    if(config.init_type_registry){
        if(!quiet) std::cout << "\nLoading Type Registry.." << std::endl;
        st = type_registry->loadTypeRegistries();
        if(!st){
            std::cerr << "Error during initialization of TypeRegistry" << std::endl;
            return false;
        }
    }

    //Init Bundle
    if(config.init_bundle){
        if(!quiet) std::cout << "\nInitializing Bundle.." << std::endl;
        bundle.reset(new Bundle);
        st = bundle->initialize(config.load_task_configs);
        if(!st){
            std::cerr << "Error during initialization of Bundle" << std::endl;
            bundle.reset();
            return false;
        }
    }

    if(!quiet) std::cout << "\nOrocosCPP initialization complete!"<<std::endl;
    return true;
}

RTT::corba::TaskContextProxy *OrocosCpp::getTaskContext(std::string name)
{
    return RTT::corba::TaskContextProxy::Create(name);
}

bool OrocosCpp::loadAllTypekitsForModel(std::string packageOrTaskModelName)
{
    return orocos_cpp::PluginHelper::loadAllTypekitsForModel(packageOrTaskModelName);
}

std::string OrocosCpp::applyStringVariableInsertions(const std::string &cnd_yaml)
{
    return libConfig::YAMLConfigParser::applyStringVariableInsertions(cnd_yaml);
}
}
