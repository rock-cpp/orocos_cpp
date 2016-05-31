#include "Deployment.hpp"
#include <boost/tokenizer.hpp>
#include <boost/filesystem.hpp>
#include <fstream>
#include <iostream>
#include "PkgConfigHelper.hpp"

using namespace orocos_cpp;

Deployment::Deployment(const std::string& name) : deploymentName(name)
{
    loadPkgConfigFile(name);
    if(!checkExecutable(name))
        throw std::runtime_error("Deployment::Error, executable for deployment " + name + " could not be found in PATH");
}

Deployment::Deployment(const std::string &cmp1, const std::string &as)
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
    
    deploymentName = defaultDeploymentName;
    try {
        loadPkgConfigFile(deploymentName);
    } catch (const std::runtime_error &e)
    {
        throw std::runtime_error("Deployment::Error, could not find pkgConfig file for deployment " + deploymentName);
    }
    if(!checkExecutable(deploymentName))
        throw std::runtime_error("Deployment::Error, executable for deployment " + deploymentName + " could not be found in PATH");

    std::string taskName = as;
    std::vector<std::string> args;
    
    if(!taskName.empty())
    {
        renameTask(defaultDeploymentName, taskName);
        renameTask(defaultDeploymentName  + "_Logger", taskName + "_Logger");
    }
}

bool Deployment::checkExecutable(const std::string& name)
{
    if(boost::filesystem::exists(name))
        return true;
    
    const char *binPath = getenv("PATH");
    if(!binPath)
    {
        throw std::runtime_error("Deployment::Internal Error, PATH is not set found.");
    }
    
    std::string binPathS = binPath;
    
    boost::char_separator<char> sep(":");
    boost::tokenizer<boost::char_separator<char> > paths(binPathS, sep);
    
    for(const std::string &path: paths)
    {
        std::string candidate = path + "/" + name;
        if(boost::filesystem::exists(candidate))
        {
            return true;
            break;
        }
    }

    return false;
}


bool Deployment::loadPkgConfigFile(const std::string& name)
{
    std::vector<std::string> pkgConfigFields;
    pkgConfigFields.push_back("typekits");
    pkgConfigFields.push_back("deployed_tasks");
    std::vector<std::string> pkgConfigValues;

    if(!PkgConfigHelper::parsePkgConfig("/orogen-" + name + ".pc", pkgConfigFields, pkgConfigValues))
        throw std::runtime_error("Deployment::Error, could not finde pkg-config file for deployment " + name );

    
    boost::char_separator<char> sep(" ");
    boost::tokenizer<boost::char_separator<char> > tkits(pkgConfigValues[0], sep);
    for(const std::string &tkit: tkits)
        typekits.push_back(tkit);

    boost::char_separator<char> sep2(",");
    boost::tokenizer<boost::char_separator<char> > tTasks(pkgConfigValues[1], sep2);
    for(const std::string &task: tTasks)
    {
        renameMap[task] = std::string();
        originalTasks.push_back(task);
        tasks.push_back(task);
        
        std::string loggerString("_Logger");
        if(task.length() > loggerString.length() && task.substr(task.length() - loggerString.length(), task.length()) == loggerString)
        {
            loggerName = task;
        }
    }

//     std::cout << "Needed Typekits :" << std::endl;
//     for(const std::string &tkit:  typekits)
//     {
//         std::cout << tkit << std::endl;
//     }
//     
//     std::cout << "Deployed Tasks :" << std::endl;
//     for(const std::string &task: originalTasks)
//     {
//         std::cout << task << std::endl;
//     }
//     
    return true;
}

const std::string& Deployment::getName() const
{
    return deploymentName;
}

const std::vector< std::string >& Deployment::getOriginalTaskNames() const
{
    return originalTasks;
}


const std::vector< std::string >& Deployment::getTaskNames() const
{
    return tasks;
}

const std::vector< std::string >& Deployment::getNeededTypekits() const
{
    return typekits;
}

void Deployment::renameTask(const std::string& orignalName, const std::string& newName)
{
    auto it = renameMap.find(orignalName);
    if(it == renameMap.end())
    {
        throw std::runtime_error("Deployment::Error, deployment " + deploymentName + " has no task " + orignalName);
    }
    
    if(!it->second.empty())
    {
        std::cout << "Deployment::Warning double renaming of task " << orignalName << ". This smells like a bug." << std::endl;
    }
    
    //set new name
    it->second = newName;

    //regenerate the task list
    tasks.clear();
    for(std::pair<std::string, std::string> p: renameMap)
    {
        if(p.second.empty())
        {
            tasks.push_back(p.first);
        }
        else
        {
            tasks.push_back(p.second);
        }
    }
}


bool Deployment::getExecString(std::string& cmd, std::vector< std::string >& args)
{
    cmd = deploymentName;
    
    args.clear();
    for(std::pair<std::string, std::string> p: renameMap)
    {
        if(!p.second.empty())
        {
//             std::cout << "Task " << p.first << " is renmaed to " << p.second << std::endl;
            args.push_back("--rename");
            args.push_back(p.first + ":" + p.second);
        }
    }
    
    return true;
}

const std::string Deployment::getLoggerName() const
{
    auto it = renameMap.find(loggerName);
    if(it == renameMap.end())
        throw std::runtime_error("Deployment::Internal Error, logger name could not be found for deployment " + deploymentName + ". Forgott the add_default_logger ?");
    
    if(it->second.empty())
        return it->first;
    
    return it->second;
}

bool Deployment::hasLogger() const
{
    return !loggerName.empty();
}
