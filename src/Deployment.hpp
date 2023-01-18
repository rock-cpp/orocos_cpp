#ifndef DEPLOYMENT_H
#define DEPLOYMENT_H

#include <vector>
#include <string>
#include <map>
#include <boost/noncopyable.hpp>

namespace orocos_cpp
{

class Deployment : public boost::noncopyable
{
private:
    bool loadPkgConfigFile(const std::string &deploymentName);
    bool checkExecutable(const std::string &name);

    std::vector<std::string> typekits;
    std::vector<std::string> tasks;

    //this map contains a map from currentName to Origininal Name
    std::map<std::string, std::string> renameMap;

    void regenerateNameVectors();

    std::string deploymentName;
    std::string loggerName;

    bool withValgrind;
    std::vector<std::string> cmdLineArgs;

public:
    Deployment(const std::string &name, int doExecutableCheck = 0);

    Deployment(const std::string &model, const std::string &as, int doExecutableCheck = 0);

    /**
     * Returns the name of the deployment
     * */
    const std::string &getName() const;

    /**
     * Returns the task names, after renaming.
     * */
    const std::vector<std::string> &getTaskNames() const;

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

    /**
     * If called, the deployment will be started within valgrind
     * */
    void runWithValgrind();

    /**
     * Set additional command line arguments. Default is empty
     * */
    void setCmdLineArgs(const std::vector<std::string> &args);
};

} //end of namespace
#endif // DEPLOYMENT_H
