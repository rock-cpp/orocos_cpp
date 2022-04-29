#include "Spawner.hpp"
#include <base/Time.hpp>
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
#include <lib_config/Bundle.hpp>
#include <signal.h>
#include <backward/backward.hpp>
#include <rtt/transports/corba/TaskContextProxy.hpp>
#include <base-logging/Logging.hpp>

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
    LOG_INFO_S << "Shutdown: trying to kill all childs";
    
    try {
        Spawner::getInstace().killAll();
        LOG_INFO_S << "Done ";
    } catch (...)
    {
        LOG_ERROR_S << "Error, during killall";
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


Spawner::ProcessHandle::ProcessHandle(Deployment *deployment, bool redirectOutputv, const std::string &logDir, const std::string textLogFileName = "") : isRunning(true), deployment(deployment)
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
            
        processName = deployment->getName();
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
        processName = deployment->getName();

        if (!textLogFileName.empty())
        {
            redirectOutput(logDir + "/" + textLogFileName + "-" + boost::lexical_cast<std::string>(getpid()) + ".txt");
        }
        else
        {
            redirectOutput(logDir + "/" + processName + "-" + boost::lexical_cast<std::string>(getpid()) + ".txt");
        }
    }
    
    //do the exec
    
    char * argv[args.size() + 2];

    argv[0] = const_cast<char *>(cmd.c_str());
    
    argv[args.size() +1] = nullptr;
    
    for(size_t i = 0; i <  args.size(); i++)
    {
        argv[i + 1] = const_cast<char *>(args[i].c_str());
    }
    
    std::stringstream ss;
    ss << "Executing ";
    for(const std::string& arg : args){
        ss << arg << " ";
    }
    LOG_INFO_S << ss.str();
    execvp(cmd.c_str(), argv);
    
    //failure case
    LOG_ERROR_S << "Start of " << cmd << " failed:" << strerror(errno);
    
    throw std::runtime_error(std::string("Start of ") + cmd + " failed:" + strerror(errno));
    
    exit(EXIT_FAILURE);
}

bool Spawner::ProcessHandle::alive() const
{
    //if it was already determined before that the process is already dead,
    //we can stop here. Otherwise waitpid would fail!
    if(!isRunning){
        return isRunning;
    }

    int status = 0;
    pid_t ret = waitpid(pid, &status, WNOHANG);
    //waitpid(): on success, returns the process ID of the child whose state has
    //changed; if WNOHANG was specified and one or more child(ren) specified by
    //pid exist, but have not yet changed state, then 0 is returned.
    //On error, -1 is returned.
    if(ret == -1 )
    {
        throw std::runtime_error(std::string("WaitPid failed ") + strerror(errno));
    }
    else if(ret == 0){
        //Not yet terminated, but also no error.. so it's still alive
        LOG_DEBUG_S << "Process " << pid << " is still running";
        return isRunning;
    }
    else if(ret == pid){
        //Its terminated. Check for status.
        if(WIFEXITED(status))
        {
            int exitStatus = WEXITSTATUS(status);
            LOG_INFO_S << "Process " << pid << " terminated normaly, return code " << exitStatus;
            isRunning = false;
        }

        if(WIFSIGNALED(status))
        {
            isRunning = false;

            int sigNum = WTERMSIG(status);

            if(sigNum == SIGSEGV)
            {

                LOG_WARN_S << "Process " << processName << " segfaulted ";
            }
            else
            {
                LOG_INFO_S << "Process " << processName << " was terminated by SIG " << sigNum;
            }
        }
    }
    else{
        std::runtime_error("waitpid returned unexpected return value "+std::to_string(ret));
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
         LOG_ERROR_S << "Error sending of SIGKILL to pid " << pid << " failed:" << strerror(errno);
    }
}

void Spawner::ProcessHandle::sendSigInt() const
{
    if(kill(pid, SIGINT))
    {
         LOG_ERROR_S << "Error sending of SIGINT to pid " << pid << " failed:" << strerror(errno);
    }
}

void Spawner::ProcessHandle::sendSigTerm() const
{
    if(kill(pid, SIGTERM))
    {
         LOG_ERROR_S << "Error sending of SIGTERM to pid " << pid << " failed:" << strerror(errno);
    }
}



Spawner::ProcessHandle &Spawner::spawnTask(const std::string& cmp1, const std::string& as, bool redirectOutput)
{
    Deployment *dpl = new Deployment(cmp1, as);
    return spawnDeployment(dpl, redirectOutput);
}

Spawner::ProcessHandle& Spawner::spawnDeployment(Deployment* deployment, bool redirectOutput, const std::string textLogFileName)
{
    if(redirectOutput && logDir.empty())
    {
        //log dir always exists if requested from bundle
        logDir = Bundle::getInstance().getLogDirectory();
    }

    ProcessHandle *handle = new ProcessHandle(deployment, redirectOutput, logDir, textLogFileName);
    
    handles.push_back(handle);

    for(const std::string &task: deployment->getTaskNames())
    {
        notReadyList.push_back(task);
    }
    
    return *handle;
}

Spawner::ProcessHandle& Spawner::spawnDeployment(const std::string& dplName, bool redirectOutput, const std::string textLogFileName)
{
    Deployment *deployment = new Deployment(dplName);

    return spawnDeployment(deployment, redirectOutput, textLogFileName);
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
            std::stringstream ss;
            ss << "Spawner::waitUntilAllReady: Error the tasks :\n";
            for(const std::string &name: notReadyList)
            {
                ss << "    " << name << "\n";
            }
            ss << "did not register at nameservice";
            LOG_ERROR_S << ss.str();
            killAll();
            throw std::runtime_error("Spawner::waitUntilAllReady: Error timeout while waiting for tasks to register at nameservice");
        }
    }
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
                LOG_DEBUG_S << "Trying to stop task " << tName;
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
        LOG_WARN_S << "Error, could not redirect cout to " << filename;
        return;
    }
    
    if(dup2(newFd, fileno(stdout))  == -1)
    {
        LOG_WARN_S << "Error, could not redirect cout to " << filename;
        return;
    }
    if(dup2(newFd, fileno(stderr))  == -1)
    {
        LOG_WARN_S << "Error, could not redirect cerr to " << filename;
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

