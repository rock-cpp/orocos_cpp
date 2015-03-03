#include "Bundle.hpp"
#include <stdlib.h>
#include <stdexcept>
#include <boost/tokenizer.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <iostream>
#include <base/Time.hpp>

Bundle * Bundle::instance;

Bundle::Bundle()
{
    const char *activeBundleC = getenv("ROCK_BUNDLE");
    if(!activeBundleC)
    {
        throw std::runtime_error("Error, no active bundle configured. Please use 'rock-bundle-default' to set one.");
    }
    activeBundle = activeBundleC;

    const char *pathsC = getenv("ROCK_BUNDLE_PATH");
    if(!pathsC)
    {
        throw std::runtime_error("Internal Error, no bundle path found.");
    }
    std::string paths = pathsC;
    
    boost::char_separator<char> sep(":");
    boost::tokenizer<boost::char_separator<char> > tokens(paths, sep);

    for(const std::string &path : tokens)
    {
        bundlePaths.push_back(path);
    }
    
    for(const std::string &path : bundlePaths)
    {
        boost::char_separator<char> sep("/");
        boost::tokenizer<boost::char_separator<char> > tokens(path, sep);
        
        std::string last;
        for(const std::string &token: tokens)
        {
            last = token;
        }
        if(last == activeBundle)
        {
            std::cout << "Found active bundle path : " << path << std::endl;
            activeBundlePath = path;
            break;
        }
    }
    
    configDir = activeBundlePath + "/config/orogen/";
}

bool Bundle::createLogDirectory()
{
    base::Time curTime = base::Time::now();
    logDir = activeBundlePath + "/logs/" + curTime.toString(base::Time::Seconds, "%Y%m%d-%H%M");

    //use other directory if log dir already exists
    std::string base = logDir;
    int i = 1;
    while(boost::filesystem::exists(logDir))
    {
        logDir = base + "." + boost::lexical_cast<std::string>(i);
        i++;
    }

    if(!boost::filesystem::create_directory(logDir))
    {
        throw std::runtime_error("Failed to create log directory :" + logDir);
    }
    
    const std::string currentPath(activeBundlePath + "/logs/current");
    
    //create symlink to current
    if(boost::filesystem::exists(currentPath))
    {
        boost::filesystem::remove(currentPath);
    }

    boost::filesystem::create_directory_symlink(logDir, currentPath);
    
    return true;
}


Bundle& Bundle::getInstance()
{
    if(!instance)
        instance = new Bundle();
    
    return *instance;
}

const std::string &Bundle::getConfigurationDirectory()
{
    return configDir;
}

const std::string& Bundle::getLogDirectory()
{
    if(logDir.empty())
        createLogDirectory();
    
    return logDir;
}
