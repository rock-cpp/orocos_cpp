#include "Deployment.hpp"
#include <boost/tokenizer.hpp>
#include <boost/filesystem.hpp>
#include <fstream>
#include <iostream>
#include "PkgConfigRegistry.hpp"
#include "PkgConfigHelper.hpp"
#include <lib_config/Bundle.hpp>
#include <base-logging/Logging.hpp>

using namespace orocos_cpp;

Deployment::Deployment(const std::string& name, bool load_pkg_config) : deploymentName(name), withValgrind(false)
{
    if(load_pkg_config){
        loadPkgConfigFile(name);
    }
    if(!checkExecutable(name))
        throw std::runtime_error("Deployment::Error, executable for deployment '" + name + "' could not be found in PATH");
}

Deployment::Deployment(const std::string &cmp1, const std::string &as, bool load_pkg_config) : withValgrind(false)
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
    if(load_pkg_config){
        try {
            loadPkgConfigFile(deploymentName);
        }
        catch (const std::runtime_error &e)
        {
            throw std::runtime_error("Deployment::Error, could not find pkgConfig file for deployment " + deploymentName);
        }
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

void Deployment::regenerateNameVectors()
{
    //regenerate the task list
    tasks.clear();
    for(std::pair<std::string, std::string> p: renameMap)
    {
        tasks.push_back(p.first);
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


bool Deployment::loadPkgConfigFile(const std::string& deploymentName)
{
    PkgConfigRegistryPtr pkgreg = PkgConfigRegistry::get();
    PkgConfig pkg;

    if(!pkgreg->getDeployment(deploymentName, pkg))
        throw std::runtime_error("PkgConfig file for deployment " + deploymentName + " was not loaded." );

    //Extract required information from PkgConfig
    std::string typekitsString, deployedTasksString;
    if(!pkg.getVariable("typekits", typekitsString)){
        throw(std::runtime_error("PkgConfig file for deployment "+deploymentName+" does not describe required typekits."));
    }
    std::vector<std::string> typekits = PkgConfigHelper::vectorizeTokenSeparatedString(typekitsString, " ");

    if(!pkg.getVariable("deployed_tasks", deployedTasksString)){
        throw(std::runtime_error("PkgConfig file for deployment "+deploymentName+" does not describe required typekits."));
    }
    std::vector<std::string> deployedTasks = PkgConfigHelper::vectorizeTokenSeparatedString(deployedTasksString, ",");

    //Populate renameMap with deployed tasks specified in PkgConfig file
    renameMap.clear();
    for(const std::string &taskName: deployedTasks)
    {
        renameMap[taskName] = taskName;
        tasks.push_back(taskName);
        
        //Identify if task is logger...
        //FIXME: Looks complicated. Does it look for the longest task name with '_Logger' suffix? WHy not simply assume '#{deploymentName}_Logger'?
        std::string loggerString("_Logger");
        if(taskName.length() > loggerString.length() && taskName.substr(taskName.length() - loggerString.length(), taskName.length()) == loggerString)
        {
            loggerName = taskName;
        }
    }

    return true;
}

const std::string& Deployment::getName() const
{
    return deploymentName;
}

const std::vector< std::string >& Deployment::getTaskNames() const
{
    return tasks;
}

const std::vector< std::string >& Deployment::getNeededTypekits() const
{
    return typekits;
}

void Deployment::renameTask(const std::string& orignalName, const std::string& newName, bool check_existence_in_rename_map)
{
    if(check_existence_in_rename_map){
        auto it = renameMap.find(orignalName);
        if(it == renameMap.end())
        {
               throw std::out_of_range("Deployment::renameTask : Error, deployment " + deploymentName + " has no task " + orignalName);
        }

        renameMap.erase(it);
    }

    renameMap[newName] = orignalName;

    //regenerate the task list
    regenerateNameVectors();
}


bool Deployment::getExecString(std::string& cmd, std::vector< std::string >& args)
{
    args.clear();

    cmd = deploymentName;
    
    //FIXME: Better handle Valgrind (and GDB and possibly more) at the software module that actually controls the processes. E.g. ProcessServer in cnd/orogen/execution
    if(withValgrind)
    {
        args.push_back("--trace-children=yes");
        args.push_back("--leak-check=full");
        args.push_back("--track-origins=yes");
        args.push_back("--log-file=" + libConfig::Bundle::getInstance().getLogDirectory() + "/" + deploymentName + "-valgrind.txt");
        args.push_back(deploymentName);
        cmd = "valgrind";
    }

    for(std::pair<std::string, std::string> p: renameMap)
    {
        if(p.second != p.first)
        {
//             std::cout << "Task " << p.second << " is renmaed to " << p.first << std::endl;
            args.push_back("--rename");
            args.push_back(p.second + ":" + p.first);
        }
    }

    for(const std::string& arg : cmdLineArgs)
        args.push_back(arg);
    
    return true;
}

const std::string Deployment::getLoggerName() const
{
    //Logger was possibly renamed.Thus, resolve new name of loggerName.
    for(std::pair<std::string, std::string> p: renameMap)
    {
        if(p.second == loggerName)
            return p.first;
    }

    //Deployment has no logger
    LOG_ERROR_S << "getLoggerName() was called on a Delployment that has no logger.";
    return "";
}

bool Deployment::hasLogger() const
{
    return !loggerName.empty();
}

void Deployment::runWithValgrind()
{
    withValgrind = true;
}

void Deployment::setCmdLineArgs(const std::vector<std::string> &args)
{
    cmdLineArgs = args;
}
