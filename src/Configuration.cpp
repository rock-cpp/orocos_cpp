#include "Configuration.hpp"

#include "YAMLConfiguration.hpp"

using namespace orocos_cpp;

ComplexConfigValue::ComplexConfigValue(): ConfigValue(COMPLEX)
{

}

ComplexConfigValue::~ComplexConfigValue()
{
    for(auto &it: values)
        delete it.second;    
}


void ComplexConfigValue::print(int level) const
{
    for(int i = 0; i < level; i++)
        std::cout << "  ";
    
    std::cout << getName() << ":" << std::endl;
    for(std::map<std::string, ConfigValue *>::const_iterator it = values.begin(); it != values.end(); it++)
    {
        it->second->print(level + 1);
    }
}

bool ComplexConfigValue::merge(const ConfigValue* other)
{
    if(other->getType() != COMPLEX)
        return false;

    if(name != other->getName())
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

void ComplexConfigValue::addValue(const std::string& name, ConfigValue* value)
{
    values.insert(std::make_pair(name, value));
}

const std::map< std::string, ConfigValue* >& ComplexConfigValue::getValues() const
{
    return values;
}


ArrayConfigValue::ArrayConfigValue(): ConfigValue(ARRAY)
{

}

ArrayConfigValue::~ArrayConfigValue()
{
    for(auto &it: values)
        delete it;
}

void ArrayConfigValue::addValue(ConfigValue* value)
{
    values.push_back(value);
}

const std::vector< ConfigValue* > ArrayConfigValue::getValues() const
{
    return values;
}

void ArrayConfigValue::print(int level) const
{
    for(int i = 0; i < level; i++)
        std::cout << "  ";

    std::cout << name << ":" << std::endl;
    for(const ConfigValue *v : values)
    {
        v->print(level + 1);
    }
}

bool ArrayConfigValue::merge(const ConfigValue* other)
{
    if(other->getType() != ARRAY)
        return false;

    if(name != other->getName())
    {
        throw std::runtime_error("Internal Error, merge between mismatching value");
    }
    
    const ArrayConfigValue *aother = dynamic_cast<const ArrayConfigValue *>(other);
    
    //we only support direct overwrite by index
    for(size_t i = 0; i < aother->values.size(); i++)
    {
        if(i < values.size())
        {
            values[i] = aother->values[i];
        }
        else
        {
            values.push_back(aother->values[i]);
        }
    }
    
    return true;
}


SimpleConfigValue::SimpleConfigValue(const std::string &v): ConfigValue(SIMPLE), value(v)
{

}

SimpleConfigValue::~SimpleConfigValue()
{

}

const std::string& SimpleConfigValue::getValue() const
{
    return value;
}

void SimpleConfigValue::print(int level) const
{
    for(int i = 0; i < level; i++)
        std::cout << "  ";
    std::cout << name << " : " << value << std::endl;
}

bool SimpleConfigValue::merge(const ConfigValue* other)
{
    if(other->getType() != SIMPLE)
        return false;
    
    if(name != other->getName())
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

const ConfigValue::Type& ConfigValue::getType() const
{
    return type;
}

const std::string& ConfigValue::getName() const
{
    return name;
}

void ConfigValue::setName(const std::string& newName)
{
    name = newName;
}

void Configuration::print()
{
    std::cout << "Configuration name is : " << name << std::endl;
    for(std::map<std::string, ConfigValue *>::const_iterator it = values.begin(); it != values.end(); it++)
    {
        it->second->print(1);
    }
}

Configuration::Configuration(const std::string& name) : name(name)
{

}

Configuration::~Configuration()
{
    for(auto &it: values)
        delete it.second;
}

const std::map< std::string, ConfigValue* >& Configuration::getValues() const
{
    return values;
}

void Configuration::addValue(const std::string& name, ConfigValue* value)
{
    values.insert(std::make_pair(name, value));
}

const std::string& Configuration::getName() const
{
    return name;
}

bool Configuration::fillFromYaml(const std::string& yml)
{
    values.clear();
    YAMLConfigParser parser;
    return parser.parseYAML(*this, parser.applyStringVariableInsertions(yml));
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
