#define once

#include <string>
#include <map>

namespace orocos_cpp
{

class TypeRegistry
{
    TypeRegistry();
    
    bool loadTypkekit(const std::string &typeName);
    void registerAtRTT();    
    bool loadTypelist();
        
    std::map<std::string, std::string> typeToTypekit;
    
    static TypeRegistry *instance;
public:
    static TypeRegistry *getInstance();

    bool getTypekitDefiningType(const std::string &typeName, std::string &typekitName);
};

}
