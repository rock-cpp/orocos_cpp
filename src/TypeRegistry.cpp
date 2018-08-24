#include "TypeRegistry.hpp"
#include "PkgConfigHelper.hpp"
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <iostream>
#include <fstream>
#include <typelib/registry.hh>
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

bool TypeRegistry::loadTypelist()
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
                        boost::replace_last(typeKitPath, "tlb", "typelist");
                        
                        std::ifstream in(typeKitPath);
                        if(in.bad())
                        {
                            throw std::runtime_error("Failed to open file " + file.filename().string());
                        }
                        
                        std::string line;
                        while(!in.eof())
                        {
                            std::getline(in, line);
                            size_t e = line.find_first_of(' ');
                            if(e == std::string::npos)
                                continue;
                            
                            std::string typeKitName = result.at(1);
                            std::string typeName = line.substr(0, e);
//                             std::cout << "Adding " <<  typeName << " to TK " << typeKitName << std::endl;
                            
                            typeToTypekit.insert(std::make_pair(typeName, typeKitName));
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

bool TypeRegistry::loadTypeRegistries()
{
    const std::string path = "../../install/share/orogen/";
    for(auto it = boost::filesystem::directory_iterator(path); it != boost::filesystem::directory_iterator(); it++)
    {
        const std::string filename = it->path().filename().string();
        const std::size_t ext_pos = filename.find_last_of(".");
        if (!filename.substr(ext_pos + 1).compare("tlb"))
        {
            Typelib::Registry* registry = new Typelib::Registry();
            if (!loadRegistry(path + filename, registry))
            {
                std::cerr << "could not load tlb path: " << path << std::endl;
                continue;
            }
            taskNameToRegistries.insert(std::make_pair(filename.substr(0, ext_pos), registry));
        }
    }
    if (taskNameToRegistries.empty())
        return false;
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

bool TypeRegistry::getStateID(const std::string &task_model_name, std::string &state_name, unsigned& id) const
{
    const std::size_t pos = task_model_name.find_first_of(":");
    auto task_it = taskNameToRegistries.find(task_model_name.substr(0, pos));
    if (task_it == taskNameToRegistries.end())
        return false;

    const std::string type_name = "/" + task_model_name.substr(0, pos) + "/" +
        task_model_name.substr(pos + 2) + "_STATES";
        
    if (!task_it->second->has(type_name))
        return false;
    
    const Typelib::Enum* states = (Typelib::Enum*) task_it->second->get(type_name);
    std::map<std::string, int> state_map = states->values();
    
    auto state_it = state_map.find(task_model_name.substr(pos + 2) + "_" + state_name);
    if (state_it == state_map.end())
        return false;
    
    id = state_it->second;
    
    return true;
}

}
