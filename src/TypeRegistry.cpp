#include "TypeRegistry.hpp"
#include "PkgConfigHelper.hpp"
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <iostream>
#include <fstream>
#include <typelib/registry.hh>
#include <typelib/registryiterator.hh>
#include <typelib/pluginmanager.hh>
#include <typelib/typemodel.hh>
#include <typelib/importer.hh>

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
    bool loadedAll = true;
    const char *pkgCfgPaths = getenv("PKG_CONFIG_PATH");
    
    if(!pkgCfgPaths)
    {
        return false;
    }
    
    std::vector<std::string> paths;
    boost::split(paths, pkgCfgPaths, boost::is_any_of(":"));

    for(const std::string& path : paths)
    {
        if(!boost::filesystem::exists(boost::filesystem::path(path)))
        {
            std::cerr << "skipping nonexisting pkg-config path: " << path << std::endl;
            continue;
        }
        
        for(auto it = boost::filesystem::directory_iterator(path); it != boost::filesystem::directory_iterator(); it++)
        {
            const boost::filesystem::path file = it->path();
            
            if(file.filename().string().find("typekit") != std::string::npos)
            {                
                // be aware of order of parsed fields
                std::vector<std::string> result, fields{"prefix", "project_name", "type_registry"};
                if(PkgConfigHelper::parsePkgConfig(file.filename().string(), fields, result))
                {
                    if(PkgConfigHelper::solveString(result.at(2), "${prefix}", result.at(0)))
                    {
                        std::string typeKitPath = result.at(2);

                        // Load any states from the tlb to taskStateToID;
                        if (!loadStateToIDMapping(typeKitPath))
                        {
                            throw std::runtime_error("Failed to open file " + file.filename().string());
                        }

                        boost::replace_last(typeKitPath, "tlb", "typelist");
                        
                        // Load types to typeToTypekit.
                        if(!loadTypeToTypekitMapping(typeKitPath, result.at(1)))
                        {
                            throw std::runtime_error("Failed to open file " + file.filename().string());
                        }
                    }
                    else
                    {
                        std::cerr << "error: couldn't solve pkg-config strings from file " << file.string() << std::endl;
                        loadedAll &= false;
                    }
                }
                else
                {
                    std::cerr << "error: couldn't parse pkg-config fields from file " << file.string() << std::endl;
                    loadedAll &= false;
                }
                
            }
            
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
