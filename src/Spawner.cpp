#include "Spawner.hpp"
#include <base/time.h>
#include <unistd.h>
#include <stdexcept>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <iostream>
#include <stdio.h>
#include <string.h>

Spawner::ProcessHandle::ProcessHandle(const std::string& cmd, const std::vector<std::string> &args) : isRunning(true)
{
    pid = fork();
    
    if(pid < 0)
    {
        throw std::runtime_error("Fork Failed");
    }
    
    //we are the parent
    if(pid != 0)
        return;
    
    
    //child, do the exec
    
    char * argv[args.size() + 2];

    argv[0] = const_cast<char *>(cmd.c_str());
    
    argv[args.size() +1] = nullptr;
    
    for(size_t i = 0; i <  args.size(); i++)
    {
        argv[i + 1] = const_cast<char *>(args[i].c_str());
    }
    
    execvp(cmd.c_str(), argv);
    
    //failure case
    std::cout << "Start of " << cmd << " failed:" << strerror(errno) << std::endl;
    
    
    exit(EXIT_FAILURE);
}

bool Spawner::ProcessHandle::alive() const
{
    int status = 0;
    pid_t ret = waitpid(pid, &status, WNOHANG);
    
    if(ret < 0 )
        throw std::runtime_error("WaitPid failed ");
    
    if(!status)
    {
        return isRunning;
    }
    
    if(WIFEXITED(status))
    {
        int exitStatus = WEXITSTATUS(status);
        std::cout << "Process " << pid << " terminated normaly, return code " << exitStatus << std::endl;
        isRunning = false;
    }
    
    if(WIFSIGNALED(status))
    {
        isRunning = false;
        
        int sigNum = WTERMSIG(status);
        
        if(sigNum == SIGSEGV)
        {
            std::cout << "Process " << pid << " segfaulted " << std::endl;            
        }
        else
        {
            std::cout << "Process " << pid << " was terminated by SIG " << sigNum << std::endl;                        
        }
    }
    
    return isRunning;
}

void Spawner::ProcessHandle::sendSigKill() const
{
    if(kill(pid, SIGKILL))
    {
         std::cout << "Error sending of SIGKILL to pid " << pid << " failed:" << strerror(errno) << std::endl;
    }
}

void Spawner::ProcessHandle::sendSigTerm() const
{
    if(kill(pid, SIGTERM))
    {
         std::cout << "Error sending of SIGTERM to pid " << pid << " failed:" << strerror(errno) << std::endl;
    }
}


Spawner::ProcessHandle &Spawner::spawnTask(const std::string& cmp1, const std::string& as)
{
    //cmp1 is expected in the format "module::TaskSpec"
    std::string::size_type pos = cmp1.find_first_of(":");
    
    if(pos == std::string::npos || cmp1.at(pos +1) != ':')
    {
        throw std::runtime_error("given component name " + cmp1 + " is not in the format 'module::TaskSpec'");
    }
    
    std::string moduleName = cmp1.substr(0, pos);
    
    std::cout << "module name " << moduleName << std::endl;

    std::string taskModelName = cmp1.substr(pos + 2, cmp1.size()) ;
    
    std::cout << "task model name " << taskModelName << std::endl;
    
    std::string defaultDeploymentName = "orogen_default_" + moduleName + "__" + taskModelName;
    
    std::cout << "executable name " << defaultDeploymentName << std::endl;
    
    //FIXME check if executable exists

    std::string taskName = as;
    std::vector<std::string> args;
    
    if(taskName.empty())
    {
        taskName = defaultDeploymentName;
    }
    else
    {
        args.push_back("--rename");
        args.push_back(defaultDeploymentName + ":" + taskName);
        args.push_back("--rename");
        args.push_back(defaultDeploymentName  + "_Logger:" + taskName + "_Logger");
    }
    
    ProcessHandle *handle = new ProcessHandle(defaultDeploymentName, args);

    handles.push_back(handle);
    
    return *handle;
}

bool Spawner::checkAllProcesses()
{
    bool allOk = true;
    for(ProcessHandle *handle : handles)
    {
        if(!handle->alive())
        {
            allOk = false;
            //we do not break here, so that we can 
            //collect signals from any other process
        }
    }
    return allOk;
}

void Spawner::killAll()
{
    //ask all processes to terminate
    for(ProcessHandle *handle : handles)
    {
        if(handle->alive())
        {
            handle->sendSigTerm();
        }
    }
    
    base::Time waitTime = base::Time::fromMilliseconds(100);
    base::Time startTime = base::Time::now();
    
    bool allDead = false;
    
    //wait until they terminated
    while(!allDead && base::Time::now() - startTime < waitTime)
    {
        allDead = false;
        for(ProcessHandle *handle : handles)
        {
            if(handle->alive())
            {
                allDead = true;
            }
        }
    }
    
    //someone just won't terminate... send sigkill
    if(!allDead)
    {
        for(ProcessHandle *handle : handles)
        {
            if(handle->alive())
            {
                handle->sendSigKill();
            }
        }
    }
}

