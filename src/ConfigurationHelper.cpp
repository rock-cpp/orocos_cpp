#include "ConfigurationHelper.hpp"

#include <yaml-cpp/yaml.h>
#include <fstream>

ComplexConfigValue::ComplexConfigValue(): ConfigValue(COMPLEX)
{

}

bool ComplexConfigValue::merge(const ConfigValue* other)
{
    if(other->type != COMPLEX)
        return false;

    if(name != other->name)
    {
        throw std::runtime_error("Internal Error, merge between mismatching value");
    }

    const ComplexConfigValue *cother = dynamic_cast<const ComplexConfigValue *>(other);
    
    for(std::map<std::string, ConfigValue *>::const_iterator it = cother->values.begin(); it != cother->values.end(); it++)
    {
        std::map<std::string, ConfigValue *>::iterator entry = values.find(it->first);
        if(entry != values.end())
        {
            if(!entry->second->merge(it->second))
                return false;
        }
        else
        {
            values.insert(*it);
        }
    }    
    return true;
}


SimpleConfigValue::SimpleConfigValue(): ConfigValue(SIMPLE)
{

}

bool SimpleConfigValue::merge(const ConfigValue* other)
{
    if(other->type != SIMPLE)
        return false;
    
    if(name != other->name)
    {
        throw std::runtime_error("Internal Error, merge between mismatching value");
    }
    
    const SimpleConfigValue *sother = dynamic_cast<const SimpleConfigValue *>(other);
    value = sother->value;
    
    return true;
}

ConfigValue::ConfigValue(Type t) : type(t)
{
    
}

ConfigValue::~ConfigValue()
{

}


void displayMap(const YAML::Node &map, int level = 0);

void printNode(const YAML::Node &node, int level = 0)
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

ConfigValue *getConfigValue(const YAML::Node &node);

bool insetMapIntoArray(const YAML::Node &map, std::map<std::string, ConfigValue *> &array)
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
        
        val->name = memberName;

        array.insert(std::make_pair(memberName, val));
    }
    return !array.empty();
}

ComplexConfigValue *getMap(const YAML::Node &map)
{
    ComplexConfigValue *mapValue = new ComplexConfigValue;
    if(!insetMapIntoArray(map, mapValue->values))
    {
        delete mapValue;
        return NULL;
    }

    return mapValue;
}

ConfigValue *getConfigValue(const YAML::Node &node)
{
    switch(node.Type())
    {
        case YAML::NodeType::Scalar:
        {
            std::string value;
            if(node.GetScalar(value))
            {
//                 std::cout << "a Scalar: "<< value << std::endl;
                SimpleConfigValue *conf = new SimpleConfigValue;
                conf->value = value;
                return conf;
            }
            else
                std::cout << "a Scalar: without value " << std::endl;
        }
            break;
        case YAML::NodeType::Sequence:
            std::cout << "a Sequence: " << node.Tag() << std::endl;
            //Error for now
            return NULL;
            break;
        case YAML::NodeType::Map:
        {
//             std::cout << "a Map: " << node.Tag() << std::endl;
            ComplexConfigValue *mapValue = new ComplexConfigValue;
            if(!insetMapIntoArray(node, mapValue->values))
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

void displayMap(const YAML::Node &map, int level)
{
    for(YAML::Iterator it = map.begin(); it != map.end(); it++)
    {
        for(int i = 0; i < level; i++)
            std::cout << "  ";
        
        std::string value;
        it.first() >> value;
        std::cout << "Value of first " << value << std::endl;
        printNode(it.second(), level + 1);
    }
    
}

void displayConfig(ConfigValue *val, int level)
{
    for(int i = 0; i < level; i++)
        std::cout << "  ";
    switch(val->type)
    {
        case ConfigValue::SIMPLE:
        {
            SimpleConfigValue *s = dynamic_cast<SimpleConfigValue *>(val);
            std::cout << s->name << " : " << s->value << std::endl;
        }
            break;
        case ConfigValue::COMPLEX:
        {
            ComplexConfigValue *c = dynamic_cast<ComplexConfigValue *>(val);
            std::cout << val->name << ":" << std::endl;
            for(std::map<std::string, ConfigValue *>::const_iterator it = c->values.begin(); it != c->values.end(); it++)
            {
                displayConfig(it->second, level + 1);
            }
        }
        break;
    }
}

void displayConfiguration(const Configuration &conf)
{
    std::cout << "Configuration name is : " << conf.name << std::endl;
    for(std::map<std::string, ConfigValue *>::const_iterator it = conf.values.begin(); it != conf.values.end(); it++)
    {
        displayConfig(it->second, 1);
    }
}

bool Configuration::merge(const Configuration& other)
{
    for(std::map<std::string, ConfigValue *>::const_iterator it = other.values.begin(); it != other.values.end(); it++)
    {
        std::map<std::string, ConfigValue *>::iterator entry = values.find(it->first);
        if(entry != values.end())
        {
            if(!entry->second->merge(it->second))
                return false;
        }
        else
        {
            values.insert(*it);
        }
    }
    
    return true;
}

bool ConfigurationHelper::loadConfigFile(const std::string& path)
{
    std::ifstream fin(path.c_str());
    YAML::Parser parser(fin);

    YAML::Node doc;
    
    Configuration curConfig;
    bool hasConfig = false;
    
    subConfigs.clear();
    
    while(parser.GetNextDocument(doc)) {
        //check for new configuration
        if(doc.Type() == YAML::NodeType::Scalar)
        {
            std::string value;
            doc >> value;

            std::cout << "Root is a scalar " << value << std::endl;

            std::string searched("name:");
            if(!value.compare(0, searched.size(), searched))
            {
                if(hasConfig)
                {
                    subConfigs.insert(std::make_pair(curConfig.name, curConfig));
                }
                
                hasConfig = true;
                
                curConfig.name = value.substr(searched.size(), value.size());
                curConfig.values.clear();

                std::cout << "Found new configuration " << curConfig.name << std::endl;
            } else
            {
                std::cout << "Error, this should not happen" << std::endl;
            }
        }
        
        //we need to ignore the first map
        if(doc.Type() == YAML::NodeType::Map)
        {
            if(!insetMapIntoArray(doc, curConfig.values))
            {
                std::cout << "Warning, could not parse config" << std::endl;
            }
        }
        
//         std::cout << "Found a node" << std::endl;
//         printNode(doc);
    }
    
    if(hasConfig)
    {
        subConfigs.insert(std::make_pair(curConfig.name, curConfig));
    }

    
    for(std::map<std::string, Configuration>::const_iterator it = subConfigs.begin(); it != subConfigs.end(); it++)
    {
        std::cout << "Cur conf \"" << it->first << "\"" << std::endl;
        displayConfiguration(it->second);
    }
    
    
    
    return true;
}

bool ConfigurationHelper::applyConfig(RTT::TaskContext* context, const std::vector< std::string >& names)
{
    if(names.empty())
        throw std::runtime_error("Error given config array was empty");
    
    std::map<std::string, Configuration>::const_iterator entry = subConfigs.find(names.front());

    if(entry == subConfigs.end())
    {
        std::cout << "Error, config " << names.front() << " not found " << std::endl;
        std::cout << "Known configs:" << std::endl;
        for(std::map<std::string, Configuration>::const_iterator it = subConfigs.begin(); it != subConfigs.end(); it++)
        {
            std::cout << "    \"" << it->first << "\"" << std::endl;
        }
        return false;
    }
    
    //first we merge the configurations
    Configuration final = entry->second;
    
    std::vector< std::string >::const_iterator it = names.begin();
    it++;
    
    for(; it != names.end(); it++)
    {
        entry = subConfigs.find(*it);

        if(entry == subConfigs.end())
        {
            std::cout << "Error, merge failed config " << *it << " not found " << std::endl;
            return false;
        }

        if(!final.merge(entry->second))
            return false;
    }    
    
    std::cout << "Resulting config is :" << std::endl;
    displayConfiguration(final);
    
    return true;
}

