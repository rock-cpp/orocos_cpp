#define once

#include <string>
#include <map>

class TypeRegistry
{
    std::map<std::string, std::string> typeToTypekit;
public:
    TypeRegistry();
    
    bool loadTypelist();
    
    bool getTypekitDefiningType(const std::string &typeName, std::string &typekitName);
};


