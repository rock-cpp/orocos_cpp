#include <memory>
#include <rtt/typelib/TypelibMarshallerBase.hpp>
#include <orocos_cpp/CorbaNameService.hpp>
#include <stdlib.h>

#include <orocos_cpp_base/OrocosHelpers.hpp>
#include "orocos_cpp.hpp"
#include "NameService.hpp"
#include <lib_config/Configuration.hpp>
#include <lib_config/TypelibConfiguration.hpp>

using namespace orocos_cpp;


RTT::base::InputPortInterface* createInputPort(RTT::TaskContext* task_context, RTT::types::TypeInfo const * type, ::std::string const & port_name) {
    std::string name = port_name + "_input";
    RTT::base::PortInterface *pi = task_context->getPort(port_name);
    if (pi) {
        // Port exists. Returns success.
        RTT::log(RTT::Info) << "Port " << port_name << " is already registered." << RTT::endlog();
        return 0;
    }
    /* Create port */
    RTT::base::InputPortInterface *new_port = type->inputPort(name);
    if (!new_port) {
        RTT::log(RTT::Error) << "An error occurred during port generation." << RTT::endlog();
            return NULL;
    }
    task_context->ports()->addEventPort(new_port->getName(), *(new_port) );
    return new_port;
}

bool is_number(const std::string& s)
{
    return !s.empty() && std::find_if(s.begin(),
        s.end(), [](unsigned char c) { return !std::isdigit(c); }) == s.end();
}


std::shared_ptr<libConfig::ConfigValue> extract_field(const std::shared_ptr<libConfig::ConfigValue> data, const std::vector<std::string>& path)
{
    //No path specified --> return whole type
    if(path.size() == 0)
        return data;

    std::shared_ptr<libConfig::ConfigValue> parent = data;
    std::string prev_field = "";
    for(const std::string& field : path){
        if(field != ""){
            if(is_number(field)){
                std::shared_ptr<libConfig::ArrayConfigValue> aparent = std::dynamic_pointer_cast<libConfig::ArrayConfigValue>(parent);
                if(!aparent)
                    throw std::runtime_error("Tried to access element index "+field+" of data "+ parent->getCxxTypeName()+", but data is not an array.");

                int idx = std::stoi(field);
                if(idx < 0){
                    idx = aparent->getValues().size()-idx;
                }

                if((int) aparent->getValues().size() <= idx){
                    std::cout << "WARNING: Trying to access index "+std::to_string(idx)+" of field "+prev_field+", but the field has only the size of "+std::to_string(aparent->getValues().size());
                    throw std::runtime_error("Invalid field access");
                }
                parent = aparent->getValues().at(idx);

            }else{
                std::shared_ptr<libConfig::ComplexConfigValue> cparent =  std::dynamic_pointer_cast<libConfig::ComplexConfigValue>(parent);
                if(!cparent)
                    throw std::runtime_error("Tried to access field "+field+" of data "+ parent->getCxxTypeName()+", but data is not a complex structure.");
                parent = cparent->getValues().at(field);
            }
        }
        prev_field = field;
    }
    return parent;
}



int main(int argc, char **argv)
{

    std::unique_ptr<orocos_cpp::NameService> ns = std::make_unique<orocos_cpp::CorbaNameService>();

    if (!ns->connect()) {
        std::cout << "Could not connect to Nameserver " << std::endl;
        return 0;
    }

    orocos_cpp::OrocosCppConfig config;
    config.load_all_packages = false;
    config.load_typekits = true;
    config.package_initialization_whitelist = {"base", "base-types"};
    std::shared_ptr<orocos_cpp::OrocosCpp> orocos = std::make_shared<orocos_cpp::OrocosCpp>();
    orocos->initialize(config, false);

    RTT::TaskContext* localtask = new RTT::TaskContext("dummy");
    RTT::DataFlowInterface df;

    std::map<std::string, RTT::base::InputPortInterface*> input_ports;

    std::vector<std::string> tasks = ns->getRegisteredTasks();
    for (const std::string &tname : tasks) {
        RTT::TaskContext* tc = ns->getTaskContext(tname);
        if (tc) {
            for (auto& portname : tc->ports()->getPortNames()) {
                RTT::base::OutputPortInterface* portInterfacePtr = dynamic_cast<RTT::base::OutputPortInterface*>(tc->getPort(portname));
                if (portInterfacePtr) {
                    std::string type = portInterfacePtr->getTypeInfo()->getTypeName();
                    // if (type == "/base/samples/Joints") {
                        // create local port the "/" is the identifier for subgroups
                        std::string identifier = tname+"/"+portname;
                        RTT::base::InputPortInterface* input_port = dynamic_cast<RTT::base::InputPortInterface *>(portInterfacePtr->antiClone());
                        df.addPort(identifier+"-reader", *input_port);
                        input_ports[identifier] = input_port;
                        // connect all ports
                        portInterfacePtr->connectTo(input_port, RTT::ConnPolicy::data());
                    // }
                }
            }
        }
    }

    //while (true) {
        for (const auto& port : input_ports) {
            if (port.second != nullptr) {
                std::string identifier = port.first;

                const RTT::types::TypeInfo* typeinfo = port.second->getTypeInfo();
                orogen_transports::TypelibMarshallerBase * transport = dynamic_cast<orogen_transports::TypelibMarshallerBase *>(typeinfo->getProtocol(orogen_transports::TYPELIB_MARSHALLER_ID));
                orogen_transports::TypelibMarshallerBase::Handle* transportHandle = transport->createSample();
                const Typelib::Registry& registry = transport->getRegistry();
                const Typelib::Type *typeptr = registry.get(transport->getMarshallingType());
                RTT::base::DataSourceBase::shared_ptr datasource = transport->getDataSource(transportHandle);

                while (port.second->read(datasource) == RTT::NewData) {
                    // convert the native, Orocos sample back to its marshallable and inspectable Typelib form
                    transport->refreshTypelibSample(transportHandle);
                    Typelib::Value val(transport->getTypelibSample(transportHandle), *(typeptr));

                    //orocos_cpp::TypeWrapper wrap(val);
                    libConfig::TypelibConfiguration tlconf;
                    std::shared_ptr<libConfig::ConfigValue> sample;
                    std::shared_ptr<libConfig::ConfigValue> field;
                    sample = tlconf.getFromValue(val);

                    //wrap.printType();
                    sample->print(std::cout);

                    if (sample->getName() == "/base/samples/joints") {
                        std::cout << "printing subtype" << std::endl;
                        field = extract_field(sample, {"elements", 0});
                        field->print(std::cout);

                    }


                    // example to get all numeric values
                    std::shared_ptr<libConfig::ConfigValue> wrapptr = tlconf.getFromValue(val);
                    std::list<std::shared_ptr<libConfig::ConfigValue>> typelist;
                    std::list<std::string> namelist;
                    typelist.push_back(wrapptr);
                    namelist.push_back(identifier);
                    while (typelist.size()) {
                        std::shared_ptr<libConfig::ConfigValue> current = typelist.front();
                        std::string name = namelist.front();
                        typelist.pop_front();
                        if (current->getType() == libConfig::ConfigValue::Type::SIMPLE) {
                            // set value
                            std::shared_ptr<libConfig::SimpleConfigValue> val = std::dynamic_pointer_cast<libConfig::SimpleConfigValue>(current);
                            double data = atof(val->getValue().c_str());
                            std::cout << name << ": " << data << std::endl;
                        } else {
                            // add other entries
                            if (current->getType() == libConfig::ConfigValue::Type::ARRAY){
                                std::shared_ptr<libConfig::ArrayConfigValue> val = std::dynamic_pointer_cast<libConfig::ArrayConfigValue>(current);
                                typelist.insert(typelist.end(), val->getValues().begin(), val->getValues().end());
                            }
                            if (current->getType() == libConfig::ConfigValue::Type::COMPLEX){
                                std::shared_ptr<libConfig::ComplexConfigValue> val = std::dynamic_pointer_cast<libConfig::ComplexConfigValue>(current);
                                for( const auto&  [key, value] : val->getValues()){
                                    typelist.push_back(value);
                                    std::string newname = name + "/" + key;
                                    namelist.push_back(newname);
                                }
                            }
                        }
                    }

                }
            }
        }
    //}


    return 0;
}
