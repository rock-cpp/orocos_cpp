#ifndef NAMESERVICE_H
#define NAMESERVICE_H

#include <vector>
#include <string>

namespace RTT
{
    class TaskContext;
};

namespace orocos_cpp
{

class NameService
{
public:
    virtual ~NameService() {};
    /**
     * Connect to the name service
     * @return true on success, false on error
     * */
    virtual bool connect() = 0;
    
    virtual std::vector<std::string> getRegisteredTasks() = 0;
    
    virtual bool isRegistered(const std::string &taskName) = 0;
    
    virtual RTT::TaskContext *getTaskContext(const std::string &taskName) = 0;
};

}//end of namespace

#endif // NAMESERVICE_H
