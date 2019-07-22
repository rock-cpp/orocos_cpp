#include "TypeRegistry.hpp"
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <iostream>
#include <fstream>
#include <typelib/registry.hh>
#include <typelib/registryiterator.hh>
#include <typelib/pluginmanager.hh>
#include <typelib/typemodel.hh>
#include <typelib/importer.hh>
#include "PkgConfigRegistry.hpp"
#include <base-logging/Logging.hpp>

namespace orocos_cpp 
{

TypeRegistry::TypeRegistry()
{
    typeToTypekit.insert(std::make_pair("int", "rtt-types"));
    typeToTypekit.insert(std::make_pair("bool", "rtt-types"));
    typeToTypekit.insert(std::make_pair("string", "rtt-types"));
    typeToTypekit.insert(std::make_pair("double", "rtt-types"));
}

bool TypeRegistry::loadTypeRegistries()
{
    PkgConfigRegistryPtr pkgreg = PkgConfigRegistry::get();

    bool loadedAll = true;

    for(const std::string& typekitName :  pkgreg->getRegisteredTypekitNames()){
        TypekitPkgConfig tpkg;
        pkgreg->getTypekit(typekitName, tpkg);

        std::string typeRegistryPath;
        if(!tpkg.typekit.getVariable("type_registry", typeRegistryPath)){
            LOG_ERROR_S << "PkgConfig file of typekit " << typekitName << " does not specify the type_registry variable";
            loadedAll = false;
            continue;
        }

        // Load any states from the tlb to taskStateToID;
        LOG_INFO_S << "Parsing Typlib file " << typeRegistryPath;
        if (!loadStateToIDMapping(typeRegistryPath))
        {
            LOG_ERROR_S << "Could not parse Typelib file " << typeRegistryPath << " which was referred to as 'type_registriy' for typekit " << typekitName << " in " << tpkg.typekit.sourceFile;
            loadedAll = false;
            continue;
        }

        // Load types to typeToTypekit.
        boost::replace_last(typeRegistryPath, "tlb", "typelist");
        LOG_INFO_S << "Parsing typelist file " <<typeRegistryPath;
        if(!loadTypeToTypekitMapping(typeRegistryPath, typekitName))
        {
            LOG_ERROR_S << "Could not parse Typelist file " << typeRegistryPath;;
            loadedAll = false;
        }
    }
    return loadedAll;
}

bool TypeRegistry::loadRegistry(const std::string &path, Typelib::Registry* registry)
{
    try{
        Typelib::PluginManager::load("tlb", path, *registry);
    } catch (const Typelib::ImportError& e){
        return false;
    }
    return true;
}

bool TypeRegistry::loadStateToIDMapping(const std::string &path)
{
    std::unique_ptr<Typelib::Registry> registry = std::unique_ptr<Typelib::Registry>(new Typelib::Registry());
    if (!loadRegistry(path, registry.get()))
    {
        std::cerr << "Could not load tlb path: " << path << std::endl;
        return false;
    }

    Typelib::RegistryIterator it = registry->begin();
    Typelib::RegistryIterator itEnd = registry->end();
    for (; it != itEnd; ++it)
    {
        // Check if type has states.
        if(it->getName().find("_STATES") == std::string::npos)
            continue;

        std::size_t length = it.getNamespace().find_last_of("/");
        const std::string nameSpace = it->getNamespace().substr(1, length - 1);

        const Typelib::Enum* states = static_cast<const Typelib::Enum*>(&*it);
        std::map<std::string, int> state_map = states->values();

        auto stateIt = state_map.begin(),
             stateItEnd = state_map.end();
        for (; stateIt != stateItEnd; ++stateIt)
        {
            std::string taskState = nameSpace + "::" + stateIt->first;
            taskStateToID.insert(std::make_pair(taskState, stateIt->second));
        }
    }

    return true;
}

bool TypeRegistry::loadTypeToTypekitMapping(const std::string &path, const std::string &typeKitName)
{
    std::ifstream in(path);
    if(in.bad())
    {
        return false;
    }

    std::string line;
    while(!in.eof())
    {
        std::getline(in, line);
        size_t e = line.find_first_of(' ');
        if(e == std::string::npos)
            continue;

        std::string typeName = line.substr(0, e);

        typeToTypekit.insert(std::make_pair(typeName, typeKitName));
    }

    return true;
}

bool TypeRegistry::getTypekitDefiningType(const std::string& typeName, std::string& typekitName)
{
    auto it = typeToTypekit.find(typeName);
    if(it == typeToTypekit.end())
        return false;
    
    typekitName = it->second;
    
    return true;
}

bool TypeRegistry::getStateID(const std::string &task_model_name, const std::string &state_name, unsigned int &id) const
{
    auto state_it = taskStateToID.find(task_model_name + "_" + state_name);
    if (state_it == taskStateToID.end())
        return false;
    
    id = state_it->second;
    
    return true;
}

}
