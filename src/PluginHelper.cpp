#include "PluginHelper.hpp"
#include <rtt/types/TypekitPlugin.hpp>

#include <boost/filesystem.hpp>
#include <boost/tokenizer.hpp>

#include <rtt/plugin/PluginLoader.hpp>
#include <base/Time.hpp>
#include "PkgConfigRegistry.hpp"
#include "PkgConfigHelper.hpp"
#include <iostream>

#define xstr(s) str(s)
#define str(s) #s

using namespace orocos_cpp;
/**
    * Cache for the needed typekits.
    */
std::map<std::string, std::vector<std::string> > PluginHelper::componentToTypeKitsMap;

std::vector< std::string > PluginHelper::getNeededTypekits(const std::string& componentName)
{

    auto it = componentToTypeKitsMap.find(componentName);
    if(it != componentToTypeKitsMap.end())
        return it->second;

    PkgConfigRegistryPtr pkgreg = PkgConfigRegistry::get();
    OrogenPkgConfig pkg;
    if(!pkgreg->getOrogen(componentName, pkg)){
        throw std::runtime_error("Could not load pkgConfig file for typekit for component " + componentName);
    }
    std::string neededTypekitsString;
    if(!pkg.tasks.getVariable("typekits", neededTypekitsString)){
        std::cerr << "Tasks-PkgConfig file for component "+ componentName + " ("+pkg.tasks.sourceFile+") is expected to define the 'typekits' variable, but it does not. Trying to load self-named typekit" << std::endl;
        neededTypekitsString = componentName;
    }

    std::vector<std::string> ret = PkgConfigHelper::vectorizeTokenSeparatedString(neededTypekitsString, " ");
    // Name of orocos-rtt typekit in RTT::types::TypekitRepository is 'rtt-types', but the rock pkg
    // config files call it as 'orocos'
    // Result is that RTT::types::TypekitRepository::hasTypekit(tk=orocos) always returns false and
    // the loading process is triggered
    // The following loop works around this naming inconsistency
    //
    // Note: It is not clear 'why' oroGen generates the tpekit name as 'orocos'. Maybe because of
    //       orocos.rb specific things? Maybe becausen there was a renaming somewhen in the past?
    for(std::string& tk : ret){
        if(tk == "orocos")
            tk = "rtt-types";
    }
    componentToTypeKitsMap.insert(std::make_pair(componentName, ret));

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

bool PluginHelper::loadTypekitAndTransports(const std::string& typekitName)
{
    //already loaded, we can just exit
    if(RTT::types::TypekitRepository::hasTypekit(typekitName))
        return true;

    //Supported transport types
    static const std::vector<std::string> knownTransports = {"corba", "mqueue", "typelib"};

    PkgConfigRegistryPtr pkgreg = PkgConfigRegistry::get();
    RTT::plugin::PluginLoader &loader(*RTT::plugin::PluginLoader::Instance());
    PkgConfig pkg;
    if(typekitName == "rtt-types" || typekitName == "orocos" )
    {
        //special case, rtt does not follow the convention below
        if(!pkgreg->getOrocosRTT(pkg)){
            throw std::runtime_error("PkgConfig for OROCOS RTT package was not loaded");
        }
        std::string libdir;
        pkg.getVariable("libdir", libdir);
        if(!loader.loadTypekits(libdir + "/orocos/gnulinux/"))
            throw std::runtime_error("Error, failed to load rtt basis typekits");

        if(!loader.loadPlugins(libdir + "/orocos/gnulinux/"))
            throw std::runtime_error("Error, failed to load rtt basis plugins");

        return true;
    }

    TypekitPkgConfig tpkg;
    if(!pkgreg->getTypekit(typekitName, tpkg))
        throw std::runtime_error("No PkgConfig file for typekit of component " + typekitName + " was loaded.");
    if(!tpkg.typekit.isLoaded()){
        throw std::runtime_error("No Typekit PkgConfig file for component " + typekitName + " was loaded.");
    }

    std::string libDir;
    tpkg.typekit.getVariable("libdir", libDir);

    //Library of typekit is named after a specific file pattern
    if(!loader.loadLibrary(libDir + "/lib" + typekitName + "-typekit-" xstr(OROCOS_TARGET) ".so"))
        throw std::runtime_error("Error, could not load typekit for component " + typekitName);

    //Load transports for typekit
    for(const std::string &transport: knownTransports)
    {
        std::map<std::string, PkgConfig>::iterator it = tpkg.transports.find(transport);
        if(it == tpkg.transports.end())
            throw std::runtime_error("No PkgConfig file was loaded for transport " + transport + " for component " + typekitName);

        pkg = it->second;
        if(!pkg.getVariable("libdir", libDir)){
            throw(std::runtime_error("PkgConfig file for transport "+transport+"  of typekit "+typekitName+" does not define a 'libdir'."));
        }

        //Library of transport for a typekit is named after a specific file pattern
        if(!loader.loadLibrary(libDir + "/lib" + typekitName + "-transport-" + transport + "-" xstr(OROCOS_TARGET) ".so"))
            throw std::runtime_error("Error, could not load transport " + transport + " for component " + typekitName);
    }

    return true;
}

//This method loads all typkits required for a task model.
bool PluginHelper::loadAllTypekitsForModel(const std::string &modelName){
    std::string componentName = modelName.substr(0, modelName.find_first_of(':'));

    std::vector<std::string> neededTks = PluginHelper::getNeededTypekits(componentName);
    bool retVal = false;
    for(const std::string &tk: neededTks)
    {
        if(RTT::types::TypekitRepository::hasTypekit(tk))
            continue;

        retVal = true;
        PluginHelper::loadTypekitAndTransports(tk);
    }
    return retVal;
}
