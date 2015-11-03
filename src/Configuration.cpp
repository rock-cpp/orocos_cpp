#include "Configuration.hpp"

#include "YAMLConfiguration.hpp"

using namespace orocos_cpp;

ComplexConfigValue::ComplexConfigValue(): ConfigValue(COMPLEX)
{

}

ComplexConfigValue::~ComplexConfigValue()
{
}


void ComplexConfigValue::print(int level) const
{
    for(int i = 0; i < level; i++)
        std::cout << "  ";
    
    std::cout << getName() << ":" << std::endl;
    for(const auto &it : values)
    {
        it.second->print(level + 1);
    }
}

bool ComplexConfigValue::merge(std::shared_ptr<ConfigValue> other)
{
    if(other->getType() != COMPLEX)
        return false;

    if(name != other->getName())
    {
        throw std::runtime_error("Internal Error, merge between mismatching value");
    }

    const ComplexConfigValue *cother = dynamic_cast<const ComplexConfigValue *>(other.get());
    
    for(const auto &it : cother->values)
    {
        std::map<std::string, std::shared_ptr<ConfigValue> >::iterator entry = values.find(it.first);
        if(entry != values.end())
        {
            if(!entry->second->merge(it.second))
                return false;
        }
        else
        {
            values.insert(it);
        }
    }    
    return true;
}


void ComplexConfigValue::addValue(const std::string &name, std::shared_ptr<ConfigValue> value)
{
    values.insert(std::make_pair(name, value));
}

const std::map< std::string, std::shared_ptr<ConfigValue> >& ComplexConfigValue::getValues() const
{
    return values;
}


ArrayConfigValue::ArrayConfigValue(): ConfigValue(ARRAY)
{

}

ArrayConfigValue::~ArrayConfigValue()
{
}

void ArrayConfigValue::addValue(std::shared_ptr<ConfigValue> value)
{
    values.push_back(value);
}

const std::vector<std::shared_ptr<ConfigValue> > ArrayConfigValue::getValues() const
{
    return values;
}

void ArrayConfigValue::print(int level) const
{
    for(int i = 0; i < level; i++)
        std::cout << "  ";

    std::cout << name << ":" << std::endl;
    for(const std::shared_ptr<ConfigValue> &v : values)
    {
        v->print(level + 1);
    }
}

bool ArrayConfigValue::merge(std::shared_ptr< ConfigValue > other)
{
    if(other->getType() != ARRAY)
        return false;

    if(name != other->getName())
    {
        throw std::runtime_error("Internal Error, merge between mismatching value");
    }
    
    const ArrayConfigValue *aother = dynamic_cast<const ArrayConfigValue *>(other.get());
    
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

bool SimpleConfigValue::merge(const std::shared_ptr< ConfigValue > other)
{
    if(other->getType() != SIMPLE)
        return false;
    
    if(name != other->getName())
    {
        throw std::runtime_error("Internal Error, merge between mismatching value");
    }
    
    const SimpleConfigValue *sother = dynamic_cast<const SimpleConfigValue *>(other.get());
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
    for(std::map<std::string, std::shared_ptr<ConfigValue> >::const_iterator it = values.begin(); it != values.end(); it++)
    {
        it->second->print(1);
    }
}

Configuration::Configuration(const std::string& name) : name(name)
{

}

Configuration::~Configuration()
{
}

const std::map< std::string, std::shared_ptr<ConfigValue> >& Configuration::getValues() const
{
    return values;
}

void Configuration::addValue(const std::string& name, std::shared_ptr<ConfigValue> value)
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
    for(std::map<std::string, std::shared_ptr<ConfigValue> >::const_iterator it = other.values.begin(); it != other.values.end(); it++)
    {
        std::map<std::string, std::shared_ptr<ConfigValue> >::iterator entry = values.find(it->first);
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
