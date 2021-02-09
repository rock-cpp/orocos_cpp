#include "Spawner.hpp"
#include <base/Time.hpp>
#include <unistd.h>
#include <stdexcept>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>
#include "CorbaNameService.hpp"
#include <lib_config/Bundle.hpp>
#include <signal.h>
#include <backward/backward.hpp>
#include <rtt/transports/corba/TaskContextProxy.hpp>

using namespace orocos_cpp;
using namespace libConfig;

struct sigaction originalSignalHandler[SIGTERM + 1];

backward::SignalHandling sh;

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


Spawner::ProcessHandle::ProcessHandle(Deployment *deploment, bool redirectOutputv, const std::string &logDir) : deployment(deploment)
{
    std::string cmd;
    std::vector< std::string > args;
    
    if(!deployment->getExecString(cmd, args))
        throw std::runtime_error("Error, could not get parameters to start deployment " + deployment->getName() );
    
    /* Block SIGINT. */
    sigset_t mask, omask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    if(sigprocmask(SIG_BLOCK, &mask, &omask))
        throw std::runtime_error("Spawner : ProcessHandle could not block SIGINT");
    
    pid = fork();
    
    if(pid < 0)
    {
        throw std::runtime_error("Fork Failed");
    }
    
    //we are the parent
    if(pid != 0)
    {
        if (setpgid(pid, pid) < 0 && errno != EACCES)
        {
            throw std::runtime_error("Spawner : ProcessHandle: Parent : Error changing process group of child");
        }
        
        if(sigprocmask(SIG_SETMASK, &omask, NULL))
        {
            throw std::runtime_error("Spawner : ProcessHandle could not unblock SIGINT");
        }
            
        processName = deploment->getName();
        return;
    }

    if(setpgid(0, 0))
    {
        throw std::runtime_error("Spawner : ProcessHandle: Child : Error could not change process group");
    }
    
    //child, redirect output
    if(redirectOutputv)
    {
        //check if directory exists, and create if not
        if(!boost::filesystem::exists(logDir))
        {
            throw std::runtime_error("Error, log directory '" + logDir + "' does not exist, but it should !");
        }
        processName = deploment->getName();
        redirectOutput(logDir + "/" + processName + "-" + boost::lexical_cast<std::string>(getpid()) + ".txt");
    }
    
    //do the exec
    
    char * argv[args.size() + 2];

    argv[0] = const_cast<char *>(cmd.c_str());
    
    argv[args.size() +1] = nullptr;
    
    for(size_t i = 0; i <  args.size(); i++)
    {
        argv[i + 1] = const_cast<char *>(args[i].c_str());
    }
    
    std::cout << "Executing " << cmd;
    for(const std::string& arg : args){
        std::cout << arg << " ";
    }
    std::cout << std::endl;
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
    
    if(ret == 0)
        return true;
    else if(ret == pid)
        return false;
    else if(ret == -1 )
        throw std::runtime_error(std::string("waitpid failed: ") + strerror(errno));
    else
        throw std::runtime_error("waitpid returned undocumented value");
}

bool Spawner::ProcessHandle::wait(int cycles, int usecs_between_cycles) const
{
    while(alive())
    {
        usleep(usecs_between_cycles);
        cycles --;
        if(cycles <= 0)
            return false;
    }
    return true;
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
    Deployment *dpl = new Deployment(cmp1, as);
    return spawnDeployment(dpl, redirectOutput);
}

Spawner::ProcessHandle& Spawner::spawnDeployment(Deployment* deployment, bool redirectOutput)
{
    if(redirectOutput && logDir.empty())
    {
        //log dir always exists if requested from bundle
        logDir = Bundle::getInstance().getLogDirectory();
    }

    // rename the logger of default deployments
    // this guarantees that every task has it's own logger
    if(deployment->getName().find("orogen_default_") == 0)
    {
        deployment->renameTask(deployment->getLoggerName(), deployment->getName() + "_Logger");
    }

    ProcessHandle *handle = new ProcessHandle(deployment, redirectOutput, logDir);
    
    handles.push_back(handle);

    for(const std::string &task: deployment->getTaskNames())
    {
        notReadyList.push_back(task);
    }
    
    return *handle;
}

Spawner::ProcessHandle& Spawner::getDeployment(const std::string& dplName)
{
    for(Spawner::ProcessHandle* h : handles)
    {
        if(h->getDeployment().getName() == dplName)
            return *h;
    }
    throw std::runtime_error(std::string("Deployment does not exist: ") + dplName);
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

bool Spawner::killDeployment(const std::string &dplName)
{
    Spawner::ProcessHandle &handle = getDeployment(dplName);

    handle.sendSigInt();
    if(!handle.wait())
    {
        std::cout << "Failed to terminate deployment '" << dplName << "', trying to kill..." << std::endl;
        handle.sendSigKill();
        if(!handle.wait())
        {
            std::cout << "Failed to kill deployment '" << dplName << "'." << std::endl;
            return false;
        }
    }
    return true;
}

void Spawner::killAll()
{
    //first we try to stop and cleanup the processes
    //ask all processes to terminate
    for(ProcessHandle *handle : handles)
    {
        for(const std::string &tName: handle->getDeployment().getTaskNames())
        {
            try {
                std::cout << "Trying to stop task " << tName << std::endl;
                RTT::corba::TaskContextProxy *proxy = RTT::corba::TaskContextProxy::Create(tName, false);
                if(proxy && proxy->isRunning())
                    proxy->stop();
            }
            catch (...)
            {
                //don't care, we want to shut down anyways
            }
        }
    }    
    
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

bool Spawner::isRunning(const Deployment* instance)
{
    for(ProcessHandle *handle: handles)
    {
        if(&(handle->getDeployment()) == instance)
        {
            return true;
        }
    }
    
    return false;
}

void Spawner::setLogDirectory(const std::string& log_folder)
{
    logDir = log_folder;
}

