#ifndef DEPLOYMENT_H
#define DEPLOYMENT_H

#include <vector>
#include <string>
#include <map>
#include <boost/noncopyable.hpp>

namespace orocoscpp{

class Deployment : public boost::noncopyable
{
private:
    bool loadPkgConfigFile(const std::string &name);
    bool checkExecutable(const std::string &name);
    
    std::vector<std::string> typekits;
    std::vector<std::string> tasks;
    std::vector<std::string> originalTasks;

    std::vector<std::string> renamedTasks;
    
    std::map<std::string, std::string> renameMap;
    
    std::string deploymentName;
    std::string loggerName;

public:
    Deployment(const std::string &name);
    
    /**
     * Returns the name of the deployment
     * */
    const std::string &getName() const;
    
    /**
     * Returns the task names, after renaming.
     * */
    const std::vector<std::string> &getTaskNames() const;
    
    /**
     * Returns the task names before the renaming operation
     * */
    const std::vector<std::string> &getOriginalTaskNames() const;
    const std::vector<std::string> &getNeededTypekits() const;
    
    /**
     * Renames a task from the deployment to the given name
     * */
    void renameTask(const std::string &orignalName, const std::string &newName);

    /**
     * Returns the command line string, needed to 
     * start the deployment, including the renaming 
     * operations.
     * */
    bool getExecString(std::string& cmd, std::vector< std::string >& args);
    
    /**
     * Returns the name of the logger for this deployment
     * */
    const std::string getLoggerName() const;
    
    /**
     * Returns if this deployment contains a logger
     * */
    bool hasLogger() const;
    
};

} //end of namespace
#endif // DEPLOYMENT_H
