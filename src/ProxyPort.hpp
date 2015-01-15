#ifndef PROXYPORT_H
#define PROXYPORT_H

#include <rtt/FlowStatus.hpp>
#include <rtt/InputPort.hpp>
#include <rtt/OutputPort.hpp>
#include "OrocosHelpers.hpp"
#include <rtt/TaskContext.hpp>

template<typename T>
class OutputProxyPort;

template<typename T>
class InputProxyPort;

template<typename T>
class ProxyReader
{
    RTT::InputPort<T> *localPort;
public:
    ProxyReader(RTT::base::OutputPortInterface *port): localPort()
    {
        if(!localPort)
            throw std::runtime_error("Error, could not create ProxyReader");
        
        RTT::TaskContext *clientTask = getClientTask();
        clientTask->addPort(*localPort);
        port->connectTo(localPort);
    }
    RTT::FlowStatus read(T &sample, bool copy_old_data = true)
    {
        return localPort->read(sample, copy_old_data);
    };
};

template<typename T>
class ProxyWriter
{
    RTT::OutputPort<T> *localPort;
public:
    ProxyWriter(RTT::base::InputPortInterface *port): localPort(dynamic_cast<RTT::OutputPort<T> * >(port->antiClone()))
    {
        if(!localPort)
            throw std::runtime_error("Error, could not create ProxyWriter");
        
        RTT::TaskContext *clientTask = getClientTask();
        clientTask->addPort(*localPort);
        localPort->connectTo(port);
    }
    
    void write(const T &sample)
    {
        localPort->write(sample);
    };
};

template<typename T>
class InputProxyPort
{
    friend class OutputProxyPort<T>;
    RTT::base::InputPortInterface *port;
    RTT::OutputPort<T> *writer;
public:
    InputProxyPort(RTT::base::PortInterface *iface): port(dynamic_cast<RTT::base::InputPortInterface *>(iface)), writer(NULL)
    {
        std::cout << "Input : Iface is " << iface << std::endl;
        if(!port)
            throw std::runtime_error("Flow interface of wrong type given");
    };

    
    RTT::OutputPort<T> &getWriter()
    {
        return getWriter(port->getDefaultPolicy());
    }
    
    RTT::OutputPort<T> &getWriter(RTT::ConnPolicy const& policy)
    {
        if(!writer)
        {
            writer = dynamic_cast<RTT::OutputPort<T> * >(port->antiClone());
            RTT::TaskContext *clientTask = getClientTask();
            clientTask->addPort(*writer);
            port->connectTo(writer, policy);
        }
        return *writer;
    };

    void removeWriter()
    {
        if(writer)
        {
            writer->disconnect();
            delete writer;
            writer = NULL;
        }
    }
    
    ~InputProxyPort()
    {
        removeWriter();
        port->disconnect();
    }
};

template<typename T>
class OutputProxyPort
{
    friend class InputProxyPort<T>;
    RTT::base::OutputPortInterface *port;
    RTT::InputPort<T> *reader;
public:
    OutputProxyPort(RTT::base::PortInterface *iface): port(dynamic_cast<RTT::base::OutputPortInterface *>(iface)), reader(NULL)
    {
        std::cout << "Output : Iface is " << iface << std::endl;
        if(!port)
            throw std::runtime_error("Flow interface of wrong type given");
    };
    
    RTT::InputPort<T> &getReader()
    {
        return getReader(RTT::ConnPolicy());
    }

    RTT::InputPort<T> &getReader(RTT::ConnPolicy const& policy)
    {
        if(!reader)
        {
            reader = dynamic_cast<RTT::InputPort<T> *>(port->antiClone());
            RTT::TaskContext *clientTask = getClientTask();
            clientTask->addPort(*reader);
            reader->connectTo(port, policy);
        }
        
        return *reader;
    };    

    void deleteReader()
    {
        if(reader)
        {
            reader->disconnect();
            delete reader;
            reader = NULL;
        }
    }
    
    bool connectTo(InputProxyPort<T> &inputPort, RTT::ConnPolicy const& policy)
    {
        return port->connectTo(inputPort.port, policy);
    };
    
    bool connectTo(InputProxyPort<T> &inputPort)
    {
        return port->connectTo(inputPort.port);
    };
    
    ~OutputProxyPort()
    {
        deleteReader();
        port->disconnect();
    }
};


#endif // PROXYPORT_H
