#include "PluginHelper.hpp"
#include <rtt/types/TypekitPlugin.hpp>
    
#include <boost/filesystem.hpp>
#include <boost/tokenizer.hpp>

#include <rtt/plugin/PluginLoader.hpp>
#include <base/Time.hpp>
#include "PkgConfigHelper.hpp"

#define xstr(s) str(s)
#define str(s) #s

std::vector< std::string > PluginHelper::getNeededTypekits(const std::string& componentName)
{
    //first we load the typekit
    std::vector<std::string> pkgConfigFields;
    pkgConfigFields.push_back("typekits");
    std::vector<std::string> pkgConfigValues;

    if(!PkgConfigHelper::parsePkgConfig(componentName + std::string("-tasks-") + xstr(OROCOS_TARGET) + std::string(".pc"), pkgConfigFields, pkgConfigValues))
        throw std::runtime_error("Could not load pkgConfig file for typekit for component " + componentName);

    boost::char_separator<char> sep(" ");
    boost::tokenizer<boost::char_separator<char> > typekits(pkgConfigValues[0], sep);

    std::vector< std::string > ret;
    for(const std::string &tk: typekits)
    {
        ret.push_back(tk);
    }
    
    return ret;
}

void PluginHelper::loadAllPluginsInDir(const std::string& path)
{
    base::Time start = base::Time::now();
    boost::filesystem::path pluginDir(path);

    int cnt = 0;
    boost::shared_ptr<RTT::plugin::PluginLoader> loader = RTT::plugin::PluginLoader::Instance();
    
    boost::filesystem::directory_iterator end_it; // default construction yields past-the-end
    for (boost::filesystem::directory_iterator it( pluginDir );
        it != end_it; it++ )
    {
        if(boost::filesystem::is_regular_file(*it))
        {
//             std::cout << "Found library " << *it << std::endl;
            loader->loadLibrary(it->path().string());
            cnt++;
        }
    }
    base::Time end = base::Time::now();

    std::cout << "Loaded " << cnt << " typekits in " << (end - start).toSeconds() << " Seconds " << std::endl; 
}

bool PluginHelper::loadTypekitAndTransports(const std::string& componentName)
{
    //already loaded, we can just exit
    if(RTT::types::TypekitRepository::hasTypekit(componentName))
        return true;
    
    std::vector<std::string> knownTransports;
    knownTransports.push_back("corba");
    knownTransports.push_back("mqueue");
    knownTransports.push_back("typelib");
    
    //first we load the typekit
    std::vector<std::string> pkgConfigFields;
    pkgConfigFields.push_back("prefix");
    pkgConfigFields.push_back("libdir");
    std::vector<std::string> pkgConfigValues;

    RTT::plugin::PluginLoader &loader(*RTT::plugin::PluginLoader::Instance());

    if(componentName == "rtt-types")
    {
        //special case, rtt does not follow the convention below
        if(!PkgConfigHelper::parsePkgConfig("orocos-rtt-" xstr(OROCOS_TARGET) ".pc", pkgConfigFields, pkgConfigValues))
            throw std::runtime_error("Could not load pkgConfig file for typekit for component " + componentName);
    
        if(!loader.loadTypekits(pkgConfigValues[0] + "/lib/orocos/gnulinux/"))
            throw std::runtime_error("Error, failed to load rtt basis typekits");

        if(!loader.loadPlugins(pkgConfigValues[0] + "/lib/orocos/gnulinux/"))
            throw std::runtime_error("Error, failed to load rtt basis plugins");
        
        return true;
    }
    
    if(!PkgConfigHelper::parsePkgConfig(componentName + std::string("-typekit-") + xstr(OROCOS_TARGET) + std::string(".pc"), pkgConfigFields, pkgConfigValues))
        throw std::runtime_error("Could not load pkgConfig file for typekit for component " + componentName);

    std::string libDir = pkgConfigValues[1];
    if(!PkgConfigHelper::solveString(libDir, "${prefix}", pkgConfigValues[0]))
        throw std::runtime_error("Internal Error while parsing pkgConfig file");
    
        
    if(!loader.loadLibrary(libDir + "/lib" + componentName + "-typekit-" xstr(OROCOS_TARGET) ".so"))
        throw std::runtime_error("Error, could not load typekit for component " + componentName);

    for(const std::string &transport: knownTransports)
    {
        if(!PkgConfigHelper::parsePkgConfig(componentName + "-typekit-" xstr(OROCOS_TARGET) ".pc", pkgConfigFields, pkgConfigValues))
            throw std::runtime_error("Could not load pkgConfig file for transport " + transport + " for component " + componentName);

        std::string libDir = pkgConfigValues[1];
        if(!PkgConfigHelper::solveString(libDir, "${prefix}", pkgConfigValues[0]))
            throw std::runtime_error("Internal Error while parsing pkgConfig file");
        
        RTT::plugin::PluginLoader &loader(*RTT::plugin::PluginLoader::Instance());
            
        if(!loader.loadLibrary(libDir + "/lib" + componentName + "-transport-" + transport + "-" xstr(OROCOS_TARGET) ".so"))
            throw std::runtime_error("Error, could not load transport " + transport + " for component " + componentName);

    }
    
    return true;
}
