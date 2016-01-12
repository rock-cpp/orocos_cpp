#define once

#include <string>
#include <map>

namespace orocos_cpp
{

class TypeRegistry
{
    std::map<std::string, std::string> typeToTypekit;
public:
    TypeRegistry();
    
    bool loadTypelist();
    
    bool getTypekitDefiningType(const std::string &typeName, std::string &typekitName);
};

}
