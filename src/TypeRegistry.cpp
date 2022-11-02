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

TypeRegistry::TypeRegistry(PkgConfigRegistryPtr pkgreg) :
    pkgreg(pkgreg), registry(new Typelib::Registry())
{
    typeToTypekit.insert(std::make_pair("int", "rtt-types"));
    typeToTypekit.insert(std::make_pair("bool", "rtt-types"));
    typeToTypekit.insert(std::make_pair("string", "rtt-types"));
    typeToTypekit.insert(std::make_pair("double", "rtt-types"));
}

TypeRegistry::TypeRegistry() : TypeRegistry(PkgConfigRegistry::get())
{
}

bool TypeRegistry::loadTypeRegistry(const std::string& typekitName, bool force)
{
    if(std::find(loadedTypekits.begin(), loadedTypekits.end(), typekitName) != loadedTypekits.end() && !force){
        return true;
    }

    //Resolve bath to TLB file
    TypekitPkgConfig tpkg;
    if(!pkgreg->getTypekit(typekitName, tpkg)){
        LOG_ERROR_S << "Could not retrieve Tpekit from PkgConfigRegistry";
        return false;
    }

    std::string typeRegistryPath;
    if(!tpkg.typekit.getVariable("type_registry", typeRegistryPath)){
        LOG_INFO_S << "PkgConfig file of typekit " << typekitName << " does not specify the type_registry variable";
        return false;
    }

    //load TLB file into
    LOG_INFO_S << "Parsing Typlib file " << typeRegistryPath;
    if (!loadTypelibRegistry(typeRegistryPath))
    {
        LOG_ERROR_S << "Could not load tlb path: " << typeRegistryPath;
        return false;
    }

    // Load any states from the tlb to taskStateToID;
    if (!loadStateToIDMapping(typeRegistryPath))
    {
        LOG_ERROR_S << "Could not parse Typelib file " << typeRegistryPath << " which was referred to as 'type_registriy' for typekit " << typekitName << " in " << tpkg.typekit.sourceFile;
        return false;
    }

    // Load types to typeToTypekit.
    boost::replace_last(typeRegistryPath, "tlb", "typelist");
    LOG_INFO_S << "Parsing typelist file " <<typeRegistryPath;
    if(!loadTypeToTypekitMapping(typeRegistryPath, typekitName))
    {
        LOG_ERROR_S << "Could not parse Typelist file " << typeRegistryPath;;
        return false;
    }

    loadedTypekits.push_back(typekitName);
    return true;
}

bool TypeRegistry::loadTypeRegistries()
{
    bool loadedAll = true;

    for(const std::string& typekitName :  pkgreg->getRegisteredTypekitNames()){
        loadedAll &= loadTypeRegistry(typekitName);
    }
    return loadedAll;
}

bool TypeRegistry::loadTypelibRegistry(const std::string &path)
{
    try{
        Typelib::PluginManager::load("tlb", path, *registry.get());
    } catch (const Typelib::ImportError& e){
        return false;
    }
    return true;
}

bool TypeRegistry::loadStateToIDMapping(const std::string &path)
{
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

bool TypeRegistry::hasType(const std::string &typeName)
{
    return registry->has(typeName, true);
}

const Typelib::Type *TypeRegistry::getTypeModel(const std::string &typeName)
{
    return registry->get(typeName);
}

bool TypeRegistry::getStateID(const std::string &task_model_name, const std::string &state_name, unsigned int &id)
{
    std::map<std::string, unsigned>::const_iterator state_it = taskStateToID.find(task_model_name + "_" + state_name);
    if (state_it == taskStateToID.end()){
        std::string typekit_name = task_model_name.substr(0, task_model_name.find(":"));
        if(!loadTypeRegistry(typekit_name))
            return false;

        state_it = taskStateToID.find(task_model_name + "_" + state_name);
        if(state_it == taskStateToID.end())
        {
            LOG_ERROR_S << "Task state " << state_name << " is missing in typekit " << typekit_name;
            return false;
        }
    }
    
    id = state_it->second;
    
    return true;
}

bool TypeRegistry::getStateName(const std::string &task_model_name, const unsigned int &id, std::string &state_name )
{
    for (const std::pair<std::string, unsigned>& elem : taskStateToID)
    {
        const std::string& tmodelname_statename = elem.first;
        if(tmodelname_statename.rfind(task_model_name, 0) == 0 && elem.second == id){
            state_name = tmodelname_statename.substr(task_model_name.size()+1);
            return true;
        }
    }
    return false;
}

}
