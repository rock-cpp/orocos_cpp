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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>
#include "CorbaNameService.hpp"
#include "Bundle.hpp"
#include <signal.h>

struct sigaction originalSignalHandler[SIGTERM];

void shutdownHandler(int signum, siginfo_t *info, void *data);

void restoreSignalHandler(int signum)
{
    if(sigaction(signum, originalSignalHandler + signum, nullptr))
    {
        throw std::runtime_error("Error, failed to reregister original signal handler");
    }    
}

void setSignalHandler(int signum)
{
    struct sigaction act;

    act.sa_sigaction = shutdownHandler;
    
    /* The SA_SIGINFO flag tells sigaction() to use the sa_sigaction field, not sa_handler. */
    act.sa_flags = SA_SIGINFO;
    
    if(sigaction(signum, &act, originalSignalHandler + signum))
    {
        throw std::runtime_error("Error, failed to register signal handler");
    }
}

void shutdownHandler(int signum, siginfo_t *info, void *data)
{
    std::cout << "Shutdown: trying to kill all childs" << std::endl;
    
    try {
        Spawner::getInstace().killAll();
        std::cout << "Done " << std::endl;
    } catch (...)
    {
        std::cout << "Error, during killall" << std::endl;
    }
    restoreSignalHandler(signum);
    raise(signum);
    
}

Spawner::Spawner()
{
    //log dir always exists if requested from bundle
    logDir = Bundle::getInstance().getLogDirectory();

    nameService = new CorbaNameService();
    nameService->connect();

    setSignalHandler(SIGINT);
    setSignalHandler(SIGQUIT);
    setSignalHandler(SIGABRT);
    setSignalHandler(SIGSEGV);
    setSignalHandler(SIGTERM);
}

Spawner& Spawner::getInstace()
{
    static Spawner *instance = nullptr;
    
    if(!instance)
    {
        instance = new Spawner();
    }
    
    return *instance;
}


Spawner::ProcessHandle::ProcessHandle(Deployment *deploment, bool redirectOutputv, const std::string &logDir) : isRunning(true), deployment(deploment)
{
    std::string cmd;
    std::vector< std::string > args;
    
    if(!deployment->getExecString(cmd, args))
        throw std::runtime_error("Error, could not get parameters to start deployment " + deployment->getName() );
    
    pid = fork();
    
    if(pid < 0)
    {
        throw std::runtime_error("Fork Failed");
    }
    
    //we are the parent
    if(pid != 0)
        return;

    //child, redirect output
    if(redirectOutputv)
    {
        //check if directory exists, and create if not
        if(!boost::filesystem::exists(logDir))
        {
            throw std::runtime_error("Error, log directory '" + logDir + "' does not exist, but it should !");
        }
        redirectOutput(logDir + "/" + cmd + "-" + boost::lexical_cast<std::string>(getpid()) + ".txt");
    }
    
    //do the exec
    
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
    
    throw std::runtime_error(std::string("Start of ") + cmd + " failed:" + strerror(errno));
    
    exit(EXIT_FAILURE);
}

bool Spawner::ProcessHandle::alive() const
{
    int status = 0;
    pid_t ret = waitpid(pid, &status, WNOHANG);
    
    if(ret < 0 )
    {
        throw std::runtime_error(std::string("WaitPid failed ") + strerror(errno));
    }
    
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

const Deployment& Spawner::ProcessHandle::getDeployment() const
{
    return *deployment;
}

void Spawner::ProcessHandle::sendSigKill() const
{
    if(kill(pid, SIGKILL))
    {
         std::cout << "Error sending of SIGKILL to pid " << pid << " failed:" << strerror(errno) << std::endl;
    }
}

void Spawner::ProcessHandle::sendSigInt() const
{
    if(kill(pid, SIGINT))
    {
         std::cout << "Error sending of SIGINT to pid " << pid << " failed:" << strerror(errno) << std::endl;
    }
}

void Spawner::ProcessHandle::sendSigTerm() const
{
    if(kill(pid, SIGTERM))
    {
         std::cout << "Error sending of SIGTERM to pid " << pid << " failed:" << strerror(errno) << std::endl;
    }
}



Spawner::ProcessHandle &Spawner::spawnTask(const std::string& cmp1, const std::string& as, bool redirectOutput)
{
    //cmp1 is expected in the format "module::TaskSpec"
    std::string::size_type pos = cmp1.find_first_of(":");
    
    if(pos == std::string::npos || cmp1.at(pos +1) != ':')
    {
        throw std::runtime_error("given component name " + cmp1 + " is not in the format 'module::TaskSpec'");
    }
    
    std::string moduleName = cmp1.substr(0, pos);
    std::string taskModelName = cmp1.substr(pos + 2, cmp1.size()) ;
    std::string defaultDeploymentName = "orogen_default_" + moduleName + "__" + taskModelName;
    
    Deployment *dpl = new Deployment(defaultDeploymentName);

    std::string taskName = as;
    std::vector<std::string> args;
    
    if(!taskName.empty())
    {
        dpl->renameTask(defaultDeploymentName, taskName);
        dpl->renameTask(defaultDeploymentName  + "_Logger", taskName + "_Logger");
    }

    return spawnDeployment(dpl, redirectOutput);
}

Spawner::ProcessHandle& Spawner::spawnDeployment(Deployment* deployment, bool redirectOutput)
{
    ProcessHandle *handle = new ProcessHandle(deployment, redirectOutput, logDir);
    
    handles.push_back(handle);

    for(const std::string &task: deployment->getTaskNames())
    {
        notReadyList.push_back(task);
    }
    
    return *handle;
}

Spawner::ProcessHandle& Spawner::spawnDeployment(const std::string& dplName, bool redirectOutput)
{
    Deployment *deploment = new Deployment(dplName);

    return spawnDeployment(deploment, redirectOutput);
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

bool Spawner::allReady()
{
    auto it = notReadyList.begin();
    for(;it != notReadyList.end(); it++)
    {
        if(nameService->isRegistered(*it))
        {
            it = notReadyList.erase(it);
        }
        
        if(it == notReadyList.end())
            break;
    }
    
    return notReadyList.empty();
}

void Spawner::waitUntilAllReady(const base::Time& timeout)
{
    base::Time start = base::Time::now();
    while(!allReady())
    {
        usleep(10000);
        
        if(base::Time::now() - start > timeout)
        {
            std::cout << "Spawner::waitUntilAllReady: Error the tasks :" << std::endl;
            for(const std::string &name: notReadyList)
            {
                std::cout << "    " << name << std::endl;
            }
            std::cout << "did not register at nameservice" << std::endl;
            killAll();
            throw std::runtime_error("Spawner::waitUntilAllReady: Error timeout while waiting for tasks to register at nameservice");
        }
    }
}

void Spawner::killAll()
{
    //ask all processes to terminate
    for(ProcessHandle *handle : handles)
    {
        if(handle->alive())
        {
            //we send a sigint here, as this should trigger a clean shutdown
            handle->sendSigInt();
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

void Spawner::sendSigTerm()
{
    //ask all processes to terminate
    for(ProcessHandle *handle : handles)
    {
        handle->sendSigTerm();
    }
}


void Spawner::ProcessHandle::redirectOutput(const std::string& filename)
{
    int newFd = open(filename.c_str(), O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if(!newFd)
    {
        std::cout << "Error, could not redirect cout to " << filename << std::endl;
        return;
    }
    
    if(dup2(newFd, fileno(stdout))  == -1)
    {
        std::cout << "Error, could not redirect cout to " << filename << std::endl;
        return;
    }
    if(dup2(newFd, fileno(stderr))  == -1)
    {
        std::cout << "Error, could not redirect cerr to " << filename << std::endl;
        return;
    }
}

std::vector< const Deployment* > Spawner::getRunningDeployments()
{
    std::vector< const Deployment* > ret;
    ret.reserve(handles.size());
    for(ProcessHandle *handle: handles)
    {
        ret.push_back(&(handle->getDeployment()));
    }
    
    return ret;
}


