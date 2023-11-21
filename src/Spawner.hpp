#ifndef SPAWNER_H
#define SPAWNER_H

#include <unistd.h>
#include <string>
#include <vector>
#include <base/Time.hpp>
#include "NameService.hpp"
#include "Deployment.hpp"
#include <boost/noncopyable.hpp>

namespace orocos_cpp
{

class Spawner : public boost::noncopyable
{
    std::string logDir;
    
    //list of tasks that were spawned, but are not yet reachable.
    std::vector<std::string> notReadyList;
    
    NameService *nameService;
    
    
    /**
     * Default constructor
     * */
    Spawner();    

public:
    class ProcessHandle
    {
        mutable bool isRunning;
        pid_t pid;
        void redirectOutput(const std::string &filename);
        std::string processName;
        
        Deployment *deployment;
    public:
        ProcessHandle(Deployment *deployment, bool redirectOutput, const std::string &logDir, std::string textLogFileName = "");
        
        const Deployment &getDeployment() const;
        bool alive() const;
        void sendSigInt() const;
        void sendSigTerm() const;
        void sendSigKill() const;
    };

public:
    
    /**
     * Singleton pattern, returns the ONE instance of
     * the spawner.
     * */
    static Spawner &getInstace();
    
    /**
     * This method spawns a default deployment matching the given componente description.
     * If a second argument is given, the task will be renamed to the given name.
     * If a third argument is given, the task text logs will be redirected to the bundle log folder.
     * The method will throw if any error occures.
     * 
     * @arg cmp1 The task model name e.g Hokuyo::Task 
     * @arg as The name unter wich the taskmodel should be registered a the nameservice
     * @arg redirectOutput flag about whether the deployment text log files should be redirected
     * @return A Process handle, which may be used to request the process status
     * */
    ProcessHandle &spawnTask(const std::string &cmp1, const std::string &as = std::string(), bool redirectOutput = true);
    
    /**
     * This method spawns a process that executes the deployment with the given name.
     * This method will throw if any error occures.
     * If a second argument is given, the task text logs will be redirected to the bundle log folder.
     * If a third argument is given then this argument will be used as the name of the text log file.
     * 
     * @arg dplName Name of the deployment executable that should be started
     * @arg redirectOutput flag about whether the deployment text log files should be redirected
     * @arg textLogFileName Name (without .txt extension) of the output deployment text log file
     * @return  A Process handle, which may be used to request the process status
     * */
    ProcessHandle &spawnDeployment(const std::string &dplName, bool redirectOutput = true, const std::string textLogFileName = "");

    /**
     * This method spawns a process that executes the given deployment.
     * This method will throw if any error occures.
     * If a second argument is given, the task text logs will be redirected to the bundle log folder.
     * If a third argument is given then this argument will be used as the name of the text log file.
     * 
     * @arg dplName The deployment that should be started. The ownership of the deployment will be taken over by the spawner.
     * @arg redirectOutput flag about whether the deployment text log files should be redirected
     * @arg textLogFileName Name (without .txt extension) of the output deployment text log file
     * @return  A Process handle, which may be used to request the process status
     * */
    ProcessHandle &spawnDeployment(Deployment *deployment, bool redirectOutput = true, const std::string textLogFileName = "");
    
    /**
     * This method checks if all spawened processes are still alive
     * @return false if any process died
     * */
    bool checkAllProcesses();

    /**
     * This method checks if all spawned tasks registered 
     * at the nameservice. 
     * */
    bool allReady();
    
    /**
     * Waits up to the given timeout for all tasks, to
     * be connectable via the nameservice.
     * Will throw an runtime error if not all tasks could
     * be reached.
     * */
    void waitUntilAllReady(const base::Time &timeout);
    
    /**
     * This method first sends a sigterm to all processes
     * and waits for the processes to terminate. If this
     * did not happen, it will send a sigkill and return.
     * */
    void killAll();
    
    /**
     * This method sends a sigterm to all child processes.
     * */
    void sendSigTerm();
    
    /**
     * Returns a vector of all deployments, that are currently running.
     * */
    std::vector<const Deployment *> getRunningDeployments();
    
    
    /**
     * Returns, if the given instance of a deployment is running.
     * */
    bool isRunning(const Deployment *instance);

    /**
     * Sets the default log directory.
     * If no log directory is set it will be determined using bundles.
     */
    void setLogDirectory(const std::string& log_folder);
    
private:
    
    std::vector<ProcessHandle *> handles;
};
}//end of namespace.

#endif // SPAWNER_H
