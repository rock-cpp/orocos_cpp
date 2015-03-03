/*
 Copyright (c) 2012, Alexander Duda, DFKI

 Class for accessing a corba name service to retrieve the IORs
 for registered Orocos Task Contexts.

 This class is thread safe.

*/

#ifndef __CORBA_NAME_SERVICE_CLIENT_HPP__
#define __CORBA_NAME_SERVICE_CLIENT_HPP__

#include <vector>
#include <string>
#include "TaskContextC.h"
#include <boost/thread/mutex.hpp>

namespace corba
{
    //Error class which is thrown by the NameServiceClient
    class NameServiceClientError :public std::runtime_error
    {
        public:
        NameServiceClientError(const std::string& what_arg):
            std::runtime_error(what_arg)
        {
        };
    };

    //Corba name service client
    class NameServiceClient
    {
        public:
            // initializes the name service client.
            // if no ip is given the local host is used
            // if no port is given the default port 2809 is used
            NameServiceClient(std::string name_service_ip="",std::string name_service_port = "");
            ~NameServiceClient();
            
            // returns all available task names which are bound to the name service
            std::vector<std::string> getTaskContextNames();

            // returns the port number of the used name service 
            std::string getPort();

            // returns the ip address of the used name service
            std::string getIp();

            // resets the name service client
            void reset(std::string const &ip="",std::string const &port="");

            // binds an object to the corba name service
            void bind(CORBA::Object_var const &obj,std::string const& name);

            // unbinds an object from the corba name service
            bool unbind(std::string const& name);

            // returns the IOR of a bind object
            std::string getIOR(std::string const& name);

            // raises an error if the namer service cannot be reached
            void validate();

            // if getTaskContextNames is called from a different thread 
            // abort is trying to canceling the call
            void abort();

        private:
            CosNaming::NamingContext_var getNameService();
            CosNaming::NamingContext_var getNameService(const std::string name_service_ip, const std::string name_service_port);

        private:
            std::string name_service_port;
            std::string name_service_ip;
            CosNaming::NamingContext_var root_context;
            boost::mutex mut;
            bool abort_flag;
    };
};

#endif
