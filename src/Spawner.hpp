#ifndef SPAWNER_H
#define SPAWNER_H

#include <unistd.h>
#include <string>
#include <vector>

class Spawner
{
public:
    class ProcessHandle
    {
        mutable bool isRunning;
        pid_t pid;
        
        void redirectOutput(const std::string &filename);
    public:
        ProcessHandle(const std::string& cmd, const std::vector<std::string> &args, bool redirectOutput);
        bool alive() const;
        void sendSigTerm() const;
        void sendSigKill() const;
    };
public:
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
     * with the given name. This method will throw is any error
     * occures.
     * 
     * @arg dplName Name of the deployment executable that should be started
     * @return  A Process handle, which may be used to request the process status
     * */
    ProcessHandle &spawnDeployment(const std::string &dplName, bool redirectOutput = true);
    
    /**
     * This method checks if all spawened processes are still alive
     * @return false if any process died
     * */
    bool checkAllProcesses();
    
    /**
     * This method first sends a sigterm to all processes
     * and waits for the processes to terminate. If this
     * did not happen, it will send a sigkill and return.
     * */
    void killAll();
private:
    
    std::vector<ProcessHandle *> handles;
};

#endif // SPAWNER_H
