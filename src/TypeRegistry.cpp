#include "TypeRegistry.hpp"
#include "PkgConfigHelper.hpp"
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <iostream>
#include <fstream>

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
    const char *pkgCfgPaths = getenv("PKG_CONFIG_PATH");
    
    if(!pkgCfgPaths)
    {
        return false;
    }
    
    std::vector<std::string> paths;
    boost::split(paths, pkgCfgPaths, boost::is_any_of(":"));

    for(const std::string& path : paths)
    {
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
                        return false;
                    }
                }
                else
                {
                    std::cerr << "error: couldn't parse pkg-config fields from file " << file.string() << std::endl;
                    return false;
                }
                
            }
            
        }
        
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

}