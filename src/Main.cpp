#include <iostream>
#include "corba.hh"
#include "corba_name_service_client.hh"

int main(int argc, char** argv)
{
    CorbaAccess::init(argc, argv);
    CorbaAccess *acc = CorbaAccess::instance();

    corba::NameServiceClient client;
    
    std::vector<std::string> taskList = client.getTaskContextNames();
    
    for(std::vector<std::string>::const_iterator it = taskList.begin(); it != taskList.end(); it++)
    {
        std::cout << "Task : " << *it << std::endl;
    }
    
    return 0;
}
