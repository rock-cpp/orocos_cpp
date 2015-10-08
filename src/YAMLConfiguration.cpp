#include "YAMLConfiguration.hpp"

using namespace orocos_cpp;

void orocos_cpp::printNode(const YAML::Node &node, int level)
{
    for(int i = 0; i < level; i++)
        std::cout << "  ";

    switch(node.Type())
    {
        case YAML::NodeType::Scalar:
        {
            std::string value;
            if(node.GetScalar(value))
            {
                std::cout << "a Scalar: "<< value << std::endl;
            }
            else
                std::cout << "a Scalar: without value " << std::endl;
        }
            break;
        case YAML::NodeType::Sequence:
            std::cout << "a Sequence: " << node.Tag() << std::endl;
            break;
        case YAML::NodeType::Map:
            std::cout << "a Map: " << node.Tag() << std::endl;
            displayMap(node, level + 1);
            break;
        case YAML::NodeType::Null:
            break;
    }
}

bool orocos_cpp::insetMapIntoArray(const YAML::Node& map, ComplexConfigValue& array)
{
    for(YAML::Iterator it = map.begin(); it != map.end(); it++)
    {
        std::string memberName;
        it.first() >> memberName;
//         std::cout << "Name : " << memberName << std::endl;
        ConfigValue *val = getConfigValue(it.second());
        
        if(!val)
        {
            std::cout << "Warning, could not get config value for " << memberName << std::endl;
            continue;
        }
        
        val->setName(memberName);

        array.addValue(memberName, val);
    }
    return !array.getValues().empty();
}


bool orocos_cpp::insetMapIntoArray(const YAML::Node &map, ArrayConfigValue &array)
{
    for(YAML::Iterator it = map.begin(); it != map.end(); it++)
    {
        std::string memberName;
        it.first() >> memberName;
//         std::cout << "Name : " << memberName << std::endl;
        ConfigValue *val = getConfigValue(it.second());
        
        if(!val)
        {
            std::cout << "Warning, could not get config value for " << memberName << std::endl;
            continue;
        }
        
        val->setName(memberName);

        array.addValue(val);
    }
    return !array.getValues().empty();
}

bool orocos_cpp::insetMapIntoArray(const YAML::Node& map, Configuration& conf)
{
    for(YAML::Iterator it = map.begin(); it != map.end(); it++)
    {
        std::string memberName;
        it.first() >> memberName;
//         std::cout << "Name : " << memberName << std::endl;
        ConfigValue *val = getConfigValue(it.second());
        
        if(!val)
        {
            std::cout << "Warning, could not get config value for " << memberName << std::endl;
            continue;
        }
        
        val->setName(memberName);

        conf.addValue(memberName, val);
    }
    return !conf.getValues().empty();

}       

ConfigValue *orocos_cpp::getConfigValue(const YAML::Node &node)
{
    switch(node.Type())
    {
        case YAML::NodeType::Scalar:
        {
            std::string value;
            if(node.GetScalar(value))
            {
//                 std::cout << "a Scalar: "<< value << std::endl;
                SimpleConfigValue *conf = new SimpleConfigValue(value);
                return conf;
            }
            else
                std::cout << "a Scalar: without value " << std::endl;
        }
            break;
        case YAML::NodeType::Sequence:
//             std::cout << "a Sequence: " << node.Tag() << std::endl;
            {
                ArrayConfigValue *values = new ArrayConfigValue();
                for(YAML::Iterator it = node.begin(); it != node.end(); it++)
                {
                    ConfigValue *curConf = orocos_cpp::getConfigValue(*it);
                    
                    values->addValue(curConf);
                }
                return values;
            }
            break;
        case YAML::NodeType::Map:
        {
//             std::cout << "a Map: " << node.Tag() << std::endl;
            ComplexConfigValue *mapValue = new ComplexConfigValue;
            if(!orocos_cpp::insetMapIntoArray(node, *mapValue))
            {
                delete mapValue;
                return NULL;
            }
            return mapValue;
            break;
        }
        case YAML::NodeType::Null:
            std::cout << "NULL" << std::endl;
            break;
    }
    return NULL;
}

ComplexConfigValue *orocos_cpp::getMap(const YAML::Node &map)
{
    ComplexConfigValue *mapValue = new ComplexConfigValue;
    if(!insetMapIntoArray(map, *mapValue))
    {
        delete mapValue;
        return NULL;
    }

    return mapValue;
}

void orocos_cpp::displayMap(const YAML::Node &map, int level)
{
    for(YAML::Iterator it = map.begin(); it != map.end(); it++)
    {
        for(int i = 0; i < level; i++)
            std::cout << "  ";
        
        std::string value;
        it.first() >> value;
        std::cout << "Value of first " << value << std::endl;
        orocos_cpp::printNode(it.second(), level + 1);
    }
    
}
