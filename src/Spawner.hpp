#ifndef SPAWNER_H
#define SPAWNER_H

#include <unistd.h>
#include <string>
#include <vector>
#include <base/Time.hpp>
#include "NameService.hpp"
#include "Deployment.hpp"

class Spawner
{
    std::string logDir;
    
    //list of tasks that were spawned, but are not yet reachable.
    std::vector<std::string> notReadyList;
    
    NameService *nameService;
    
    /**
     * Default constructor
     * */
    Spawner();    

    static Spawner *instance;
    
public:
    class ProcessHandle
    {
        mutable bool isRunning;
        pid_t pid;
        void redirectOutput(const std::string &filename);
        
        Deployment *deployment;
    public:
        ProcessHandle(Deployment *deployment, bool redirectOutput, const std::string &logDir);
        
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
     * The method will throw if any error occures.
     * 
     * @arg cmp1 The task model name e.g Hokuyo::Task 
     * @arg as The name unter wich the taskmodel should be registered a the nameservice
     * @return A Process handle, which may be used to request the process status
     * */
    ProcessHandle &spawnTask(const std::string &cmp1, const std::string &as = std::string(), bool redirectOutput = true);
    
    /**
     * This method spawns a process that executes the deployment
     * with the given name. This method will throw if any error
     * occures.
     * 
     * @arg dplName Name of the deployment executable that should be started
     * @return  A Process handle, which may be used to request the process status
     * */
    ProcessHandle &spawnDeployment(const std::string &dplName, bool redirectOutput = true);

    /**
     * This method spawns a process that executes the given 
     * deployment. This method will throw if any error
     * occures.
     * 
     * @arg dplName The deployment that should be started. The ownership of the deployment will be taken over by the spawner.
     * @return  A Process handle, which may be used to request the process status
     * */
    ProcessHandle &spawnDeployment(Deployment *deployment, bool redirectOutput = true);
    
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
    
private:
    
    std::vector<ProcessHandle *> handles;
};

#endif // SPAWNER_H
