#include "TypeRegistry.hpp"
#include <boost/filesystem.hpp>
#include <iostream>
#include <fstream>
#include <rtt/types/TypeInfoRepository.hpp>
#include <boost/bind.hpp>
#include "PluginHelper.hpp"

namespace orocos_cpp 
{

TypeRegistry *TypeRegistry::instance;
    
TypeRegistry::TypeRegistry()
{
    typeToTypekit.insert(std::make_pair("int", "rtt-types"));
    typeToTypekit.insert(std::make_pair("bool", "rtt-types"));
    typeToTypekit.insert(std::make_pair("string", "rtt-types"));
    typeToTypekit.insert(std::make_pair("double", "rtt-types"));
    
    loadTypelist();
    
    registerAtRTT();
}

TypeRegistry* TypeRegistry::getInstance()
{
    if(!instance)
        instance = new TypeRegistry();
    
    return instance;
}

void TypeRegistry::registerAtRTT()
{
    RTT::types::TypeInfoRepository *ti = RTT::types::TypeInfoRepository::Instance().get();
    ti->setAutoLoader(boost::bind(&TypeRegistry::loadTypkekit, this, _1));
}

bool TypeRegistry::loadTypkekit(const std::string& typeName)
{
    std::cout << "Type " << typeName << " requested " << std::endl;
    std::string tkName;
    if(getTypekitDefiningType(typeName, tkName))
    {
        std::cout << "TK " << tkName << " is defining the type " << typeName << std::endl;
        if(PluginHelper::loadTypekitAndTransports(tkName))
        {
            std::cout << "TK loaded" << std::endl;
            return true;
        }
    }
    return false;
}

bool TypeRegistry::loadTypelist()
{
    const char *pathsC = getenv("ROCK_PREFIX");
    if(!pathsC)
    {
        return false;
    }

    boost::filesystem::path orogenPath(std::string(pathsC) + "/../orogen");
    
    std::string ending(".typelist");
    
    for(auto it = boost::filesystem::directory_iterator(orogenPath); it != boost::filesystem::directory_iterator(); it++)
    {
        const auto file = it->path();
        std::string f = file.filename().string();
        size_t s = f.size();
        if(s < ending.size())
            continue;
        
        if(f.substr(s-ending.size(), s) == ending)
        {
            std::string typeKitName = f.substr(0, s-ending.size());
//             std::cout << "TypeKit is " << typeKitName << std::endl;
        
            std::ifstream in(it->path().string());
            if(in.bad())
            {
                throw std::runtime_error("Failed to open file " + f);
            }
            
            std::string line;
            while(!in.eof())
            {
                std::getline(in, line);
                size_t e = line.find_first_of(' ');
                if(e == std::string::npos)
                    continue;

                std::string typeName = line.substr(0, e);
//                 std::cout << "Adding " <<  typeName << " to TK " << typeKitName << std::endl;
                
                typeToTypekit.insert(std::make_pair(typeName, typeKitName));
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