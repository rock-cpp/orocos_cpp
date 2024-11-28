#include <memory>
#include <rtt/typelib/TypelibMarshallerBase.hpp>
#include <orocos_cpp/CorbaNameService.hpp>

#include <orocos_cpp_base/OrocosHelpers.hpp>
#include "orocos_cpp.hpp"
#include "NameService.hpp"
#include "TypeWrapper.hpp"

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

    OrocosHelpers::initClientTask("main5");

    RTT::TaskContext* localtask = OrocosHelpers::getClientTask();

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
                        const RTT::types::TypeInfo* typeinfo = portInterfacePtr->getTypeInfo();
                        RTT::base::InputPortInterface* input_port = createInputPort(localtask, typeinfo, identifier);
                        input_ports[identifier] = input_port;
                        // connect all ports
                        portInterfacePtr->connectTo(input_port);
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

                    orocos_cpp::TypeWrapper wrap(val);
                    wrap.printType();

                    if (wrap.getTypelibTypeName() == "/base/samples/joints") {
                        std::cout << "printing subtype" << std::endl;
                        TypeWrapper& element = wrap["elements"][0];
                        std::cout << element.toString() << std::endl;
                    }


                    // example to get all numeric values
                    std::shared_ptr<orocos_cpp::TypeWrapper> wrapptr = std::make_shared<orocos_cpp::TypeWrapper>(val);
                    std::list<std::shared_ptr<orocos_cpp::TypeWrapper>> typelist;
                    std::list<std::string> namelist;
                    typelist.push_back(wrapptr);
                    namelist.push_back(identifier);
                    while (typelist.size()) {
                        std::shared_ptr<orocos_cpp::TypeWrapper> current = typelist.front();
                        std::string name = namelist.front();
                        typelist.pop_front();
                        namelist.pop_front();
                        if (current->getTypelibValue().getType().getCategory() == Typelib::Type::Numeric) {
                            // set value
                            double data = current->toDouble();
                            std::cout << name << ": " << data << std::endl;
                        } else {
                            // add other entries
                            for (const auto& newentry : *current) {
                                typelist.push_back(newentry.second);
                                std::string newname = name + "/" + newentry.first;
                                namelist.push_back(newname);
                            }
                        }
                    }

                }
            }
        }
    //}

    
    return 0;
}
