#pragma once
#include <rtt/transports/corba/TaskContextProxy.hpp>
#include <orocos_cpp_base/ProxyPort.hpp>

namespace orocos_cpp
{

class LoggerProxyInitializer : public RTT::corba::TaskContextProxy
{
    public:
        LoggerProxyInitializer(std::string location, bool is_ior = false);

        static void initTypes();
};

class LoggerProxy : public LoggerProxyInitializer
{
    protected:

    public:
        
        static const std::string ModelName;
        
        /** TaskContext constructor for Logger
         * \param name Name of the task. This name needs to be unique to make it identifiable via nameservices.
         * \param initial_state The initial TaskState of the TaskContext. Default is Stopped state.
         */
        LoggerProxy(std::string location, bool is_ior = false);

        void synchronize();
        OutputProxyPort< boost::int32_t > state;


        RTT::Property< ::std::string > &file;
        RTT::Property< bool > &overwrite_existing_files;


        bool reportComponent(::std::string const & name);
        bool unreportComponent(::std::string const & name);
        bool reportPort(::std::string const & component, ::std::string const & port);
        bool unreportPort(::std::string const & component, ::std::string const & port);
        bool createLoggingPort(::std::string const & port_name, ::std::string const & type_name);
        bool removeLoggingPort(::std::string const & port_name);
        void clear();
        ::std::string getModelName();
        boost::int32_t __orogen_getTID();


};

}