#include "ConfigurationHelper.hpp"

#include <yaml-cpp/yaml.h>
#include <fstream>
#include <rtt/types/TypeInfo.hpp>
#include <rtt/typelib/TypelibMarshaller.hpp>
#include <rtt/base/DataSourceBase.hpp>

#include <boost/lexical_cast.hpp>
#include <rtt/OperationCaller.hpp>
#include "Bundle.hpp"

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

ArrayConfigValue::ArrayConfigValue(): ConfigValue(ARRAY)
{

}

bool ArrayConfigValue::merge(const ConfigValue* other)
{
    if(other->type != ARRAY)
        return false;

    if(name != other->name)
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
//             std::cout << "a Sequence: " << node.Tag() << std::endl;
            {
                ArrayConfigValue *values = new ArrayConfigValue();
                for(YAML::Iterator it = node.begin(); it != node.end(); it++)
                {
                    ConfigValue *curConf = getConfigValue(*it);
                    
                    values->values.push_back(curConf);
                }
                return values;
            }
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
        case ConfigValue::ARRAY:
        {
            ArrayConfigValue *a = dynamic_cast<ArrayConfigValue *>(val);
            assert(a);
            std::cout << val->name << ":" << std::endl;
            for(std::vector<ConfigValue *>::const_iterator it = a->values.begin(); it != a->values.end(); it++)
            {
                displayConfig(*it, level + 1);
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

bool ConfigurationHelper::parseStringBuffer(Configuration &curConfig, const std::string& buffer)
{
    std::stringstream fin(buffer);
    YAML::Parser parser(fin);

    YAML::Node doc;
    
    while(parser.GetNextDocument(doc)) {

        if(doc.Type() != YAML::NodeType::Map)
        {
            std::cout << "Error, configurations section should only contain yml maps" << std::endl;
            return false;
        }
        
        if(doc.Type() == YAML::NodeType::Map)
        {
            if(!insetMapIntoArray(doc, curConfig.values))
            {
                std::cout << "Warning, could not parse config" << std::endl;
            }
        }
    }
    
    return true;
}

#include <boost/filesystem.hpp>

bool ConfigurationHelper::loadConfigFile(const std::string& pathStr)
{
    using namespace boost::filesystem;
    
    path path(pathStr);

    if(!exists(path))
    {
        throw std::runtime_error(std::string("Error, could not find config file ") + path.c_str());
    }
    
    subConfigs.clear();

    //as this is non standard yml, we need to load and parse the config file first
    std::ifstream fin(path.c_str());
    std::string line;
    
    std::string buffer;
    
    Configuration curConfig;
    bool hasConfig = false;

    
    while(std::getline(fin, line))
    {
        if(line.size() >= 3 && line.at(0) == '-'  && line.at(1) == '-'  && line.at(2) == '-' )
        {
            //found new subsection
//             std::cout << "found subsection " << line << std::endl;
            
            std::string searched("--- name:");
            if(!line.compare(0, searched.size(), searched))
            {

                if(hasConfig)
                {
                    if(!parseStringBuffer(curConfig, buffer))
                        return false;
                    subConfigs.insert(std::make_pair(curConfig.name, curConfig));
                }
                
                hasConfig = true;
                
                curConfig.name = line.substr(searched.size(), line.size());
                curConfig.values.clear();

//                 std::cout << "Found new configuration " << curConfig.name << std::endl;
            } else
            {
                std::cout << "Error, sections must begin with '--- name:<SectionName>'" << std::endl;
                return false;
            }

        } else
        {
            //line belongs to the last detected section, add it to the buffer
            buffer.append(line);
            buffer.append("\n");
        }
    }
    
    if(hasConfig)
    {
        if(!parseStringBuffer(curConfig, buffer))
            return false;
        subConfigs.insert(std::make_pair(curConfig.name, curConfig));
    }

//     for(std::map<std::string, Configuration>::const_iterator it = subConfigs.begin(); it != subConfigs.end(); it++)
//     {
//         std::cout << "Cur conf \"" << it->first << "\"" << std::endl;
//         displayConfiguration(it->second);
//     }
    
    return true;
}

template <typename T>
bool applyValue(Typelib::Value &value, const SimpleConfigValue& conf)
{
    T *val = static_cast<T *>(value.getData());
    try {
        *val = boost::lexical_cast<T>(conf.value);
    } catch (boost::bad_lexical_cast bc)
    {
        std::cout << "Error, could not set value " << conf.value << " on property " << conf.name << " Bad lexical cast : " << bc.what() << std::endl;
        std::cout << " Target Type " << value.getType().getName() << std::endl;
        return false;
    }
    return true;
}

template <>
bool applyValue<uint8_t>(Typelib::Value &value, const SimpleConfigValue& conf)
{
    uint8_t *val = static_cast<uint8_t *>(value.getData());
    try {
        *val = boost::numeric_cast<uint8_t>(boost::lexical_cast<unsigned int>(conf.value));
    } catch (boost::bad_lexical_cast bc)
    {
        std::cout << "Error, could not set value " << conf.value << " on property " << conf.name << " Bad lexical cast : " << bc.what() << std::endl;
        std::cout << " Target Type " << value.getType().getName() << std::endl;
        return false;
    }
    return true;
}

template <>
bool applyValue<int8_t>(Typelib::Value &value, const SimpleConfigValue& conf)
{
    int8_t *val = static_cast<int8_t *>(value.getData());
    try {
        *val = boost::numeric_cast<int8_t>(boost::lexical_cast<int>(conf.value));
    } catch (boost::bad_lexical_cast bc)
    {
        std::cout << "Error, could not set value " << conf.value << " on property " << conf.name << " Bad lexical cast : " << bc.what() << std::endl;
        std::cout << " Target Type " << value.getType().getName() << std::endl;
        return false;
    }
    return true;
}

bool applyConfOnTypelibEnum(Typelib::Value &value, const SimpleConfigValue& conf)
{
    const Typelib::Enum *myenum = dynamic_cast<const Typelib::Enum *>(&(value.getType()));

    std::map<std::string, int>::const_iterator it = myenum->values().find(conf.value);
    
    //values are given as RUBY constants. We need to remove the ':' in front of them
    std::string enumName = conf.value.substr(1, conf.value.size());
    
    std::map<std::string, int>::const_iterator it = myenum->values().find(enumName);
    
    if(it == myenum->values().end())
    {
        std::cout << "Error : " << conf.value << " is not a valid enum name " << std::endl;
        return false;
    }
    
    if(myenum->getSize() != sizeof(int32_t))
    {
        std::cout << "Error, only 32 bit enums are supported atm" << std::endl;
        return false;
    }

    int32_t *val = static_cast<int32_t *>(value.getData());
    *val = it->second;
    
    return true;
}

bool applyConfOnTypelibNumeric(Typelib::Value &value, const SimpleConfigValue& conf)
{
    const Typelib::Numeric *num = dynamic_cast<const Typelib::Numeric *>(&(value.getType()));
    
    switch(num->getNumericCategory())
    {
        case Typelib::Numeric::Float:
            if(num->getSize() == sizeof(float))
            {
                return applyValue<float>(value, conf);
            }
            else
            {
                //double case
                return applyValue<double>(value, conf);
            }
            break;
        case Typelib::Numeric::SInt:
            switch(num->getSize())
            {
                case sizeof(int8_t):
                    return applyValue<int8_t>(value, conf);
                    break;
                case sizeof(int16_t):
                    return applyValue<int16_t>(value, conf);
                    break;
                case sizeof(int32_t):
                    return applyValue<int32_t>(value, conf);
                    break;
                case sizeof(int64_t):
                    return applyValue<int64_t>(value, conf);
                    break;
                default:
                    std::cout << "Error, got integer of unexpected size " << num->getSize() << std::endl;
                    return false;
                    break;
            }
            break;
        case Typelib::Numeric::UInt:
            //HACK typelib encodes bools as unsigned integer. Brrrrr
            std::string lowerCase = conf.value;
            std::transform(lowerCase.begin(), lowerCase.end(), lowerCase.begin(), ::tolower);
            if(lowerCase == "true")
            {
                SimpleConfigValue co2 = conf;
                co2.value = "1";
                return applyConfOnTypelibNumeric(value, co2);
            }
            if(lowerCase == "false")
            {
                SimpleConfigValue co2 = conf;
                co2.value = "0";
                return applyConfOnTypelibNumeric(value, co2);
            }
            
            switch(num->getSize())
            {
                case sizeof(uint8_t):
                    return applyValue<uint8_t>(value, conf);
                    break;
                case sizeof(uint16_t):
                    return applyValue<uint16_t>(value, conf);
                    break;
                case sizeof(uint32_t):
                    return applyValue<uint32_t>(value, conf);
                    break;
                case sizeof(uint64_t):
                    return applyValue<uint64_t>(value, conf);
                    break;
                default:
                    std::cout << "Error, got integer of unexpected size " << num->getSize() << std::endl;
                    return false;
                    break;
            }
            break;
    }
    return true;
}

bool initTyplibValueRecusive(Typelib::Value &value)
{
    switch(value.getType().getCategory())
    {
        case Typelib::Type::Array:
            //nothing to do here (I think)
            break;
        case Typelib::Type::Compound:
        {
            const Typelib::Compound &comp = dynamic_cast<const Typelib::Compound &>(value.getType());
            Typelib::Compound::FieldList::const_iterator it = comp.getFields().begin();
            for(;it != comp.getFields().end(); it++)
            {
                Typelib::Value fieldValue(((uint8_t *) value.getData()) + it->getOffset(), it->getType());
                initTyplibValueRecusive(fieldValue);
            }
        }
            break;
        case Typelib::Type::Container:
            {
                const Typelib::Container &cont = dynamic_cast<const Typelib::Container &>(value.getType());
                cont.init(value.getData());
            }
            break;
        case Typelib::Type::Enum:
            break;
        case Typelib::Type::Numeric:
            break;
        case Typelib::Type::Opaque:
            break;
        case Typelib::Type::Pointer:
            break;
        default:
            std::cout << "Warning: Init on unknown is not supported" << std::endl;
            break;
    }
    return true;    
}

bool applyConfOnTyplibValue(Typelib::Value &value, const ConfigValue& conf)
{
    switch(value.getType().getCategory())
    {
        case Typelib::Type::Array:
        {
            const ArrayConfigValue &arrayConfig = dynamic_cast<const ArrayConfigValue &>(conf);
            const Typelib::Array &array = dynamic_cast<const Typelib::Array &>(value.getType());
            const Typelib::Type &indirect = array.getIndirection();
            
            size_t arraySize = array.getDimension();
            
            if(arrayConfig.values.size() != arraySize)
            {
                std::cout << "Error: Array " << arrayConfig.name << " of properties has different size than array in config file" << std::endl;
                return false;
            }
            
            for(size_t i = 0;i < arraySize; i++)
            {
                size_t offset =  indirect.getSize() * i;
                Typelib::Value v( reinterpret_cast<uint8_t *>(value.getData()) + offset , indirect);
                if(!applyConfOnTyplibValue(v, *(arrayConfig.values[i])))
                    return false;
            }
        }
        break;
        case Typelib::Type::Compound:
        {
            const Typelib::Compound &comp = dynamic_cast<const Typelib::Compound &>(value.getType());
            Typelib::Compound::FieldList::const_iterator it = comp.getFields().begin();
            ComplexConfigValue cpx = dynamic_cast<const ComplexConfigValue &>(conf);
            for(;it != comp.getFields().end(); it++)
            {
                std::map<std::string, ConfigValue *>::const_iterator confIt = cpx.values.find(it->getName());
                Typelib::Value fieldValue(((uint8_t *) value.getData()) + it->getOffset(), it->getType());
                if(confIt == cpx.values.end())
                {
                    //even if we don't configure this one, we still need to initialize it
                    initTyplibValueRecusive(fieldValue);
                    continue;
                }

                ConfigValue *curConf = confIt->second;
                
                cpx.values.erase(it->getName());
                
                if(!applyConfOnTyplibValue(fieldValue, *curConf))
                    return false;
            }
            if(!cpx.values.empty())
            {
                std::cout << "Error :" << std::endl;
                for(std::map<std::string, ConfigValue *>::const_iterator it = cpx.values.begin(); it != cpx.values.end();it++)
                {
                    std::cout << "  " << it->first << std::endl;
                }
                std::cout << "is/are not members of " << comp.getName() << std::endl;
                return false;
            }
        }
            break;
        case Typelib::Type::Container:
            {
                const Typelib::Container &cont = dynamic_cast<const Typelib::Container &>(value.getType());
                if(cont.kind() == "/std/string")
                {
                    const SimpleConfigValue &sconf = dynamic_cast<const SimpleConfigValue &>(conf);
                    size_t chars = sconf.value.size();
                    
                    cont.init(value.getData());
                    
                    const Typelib::Type &indirect = cont.getIndirection();
                    for(size_t i = 0; i < chars; i++)
                    {
                        Typelib::Value singleChar((void *)( sconf.value.c_str() + i), indirect);
                        cont.push(value.getData(), singleChar);
                    }
                    break;
                }
                else
                {
                    if(conf.type != ConfigValue::ARRAY)
                    {
                        std::cout << "Error, YAML representation << " << conf.name << " of type " << value.getType().getName() << " is not an array " << std::endl;
                        std::cout << "Error, got container in property, but config value is not an array " << std::endl;
                        return false;
                    }
                    const ArrayConfigValue &array = dynamic_cast<const ArrayConfigValue &>(conf);
                    
                    const Typelib::Type &indirect = cont.getIndirection();
                    cont.init(value.getData());

                    std::vector<ConfigValue *>::const_iterator it = array.values.begin();

                    for(; it != array.values.end(); it++)
                    {
                        
                        //TODO check, this may be a memory leak
                        Typelib::Value v(new uint8_t[indirect.getSize()], indirect);
                        
                        if(!applyConfOnTyplibValue(v, *(*it)))
                        {
                            return false;
                        }
                        
                        cont.push(value.getData(), v);
                    }
                }
            }
            break;
        case Typelib::Type::Enum:
            return applyConfOnTypelibEnum(value, dynamic_cast<const SimpleConfigValue &>(conf));
            break;
        case Typelib::Type::Numeric:
            return applyConfOnTypelibNumeric(value, dynamic_cast<const SimpleConfigValue &>(conf));
            break;
        case Typelib::Type::Opaque:
            std::cout << "Warning, opaque is not supported" << std::endl;
            break;
        case Typelib::Type::Pointer:
            std::cout << "Warning, pointer is not supported" << std::endl;
            break;
        default:
            std::cout << "Warning, unknown is not supported" << std::endl;
            break;
    }
    return true;
}

bool ConfigurationHelper::applyConfToProperty(RTT::TaskContext* context, const std::string& propertyName, const ConfigValue& value)
{
    RTT::base::PropertyBase *property = context->getProperty(propertyName);
    if(!property)
    {
        std::cout << "Error, there is no property with the name '" << propertyName << "' in the TaskContext " << context->getName() << std::endl;
        
        RTT::PropertyBag *bag = context->properties();
        
        std::cout << "Known Properties " << std::endl;
        for(RTT::base::PropertyBase *prop: *bag)
        {
            std::cout << "Name " << prop->getName() << std::endl;
        }
        
        
        return false;
    }

    //get Typelib value
    const RTT::types::TypeInfo* typeInfo = property->getTypeInfo();
    orogen_transports::TypelibMarshallerBase *typelibTransport = dynamic_cast<orogen_transports::TypelibMarshallerBase*>(typeInfo->getProtocol(orogen_transports::TYPELIB_MARSHALLER_ID));

    //get data source
    RTT::base::DataSourceBase::shared_ptr ds = property->getDataSource();
    
    //TODO make faster by adding getType to transport
    const Typelib::Type *type = typelibTransport->getRegistry().get(typelibTransport->getMarshallingType());

    orogen_transports::TypelibMarshallerBase::Handle *handle = typelibTransport->createSample();

    uint8_t *buffer = typelibTransport->getTypelibSample(handle);
            
    Typelib::Value dest(buffer, *type);
            
    if(typelibTransport->readDataSource(*ds, handle))
    {
        //we need to do this, in case that it is an opaque
        typelibTransport->refreshTypelibSample(handle);
    }

    if(!applyConfOnTyplibValue(dest, value))
        return false;
    

    //we modified the typlib samples, so we need to trigger the opaque
    //function here, to generate an updated orocos sample
    typelibTransport->refreshOrocosSample(handle);

    //write value back
    typelibTransport->writeDataSource(*ds, handle);
    
    //destroy handle to avoid memory leak
    typelibTransport->deleteHandle(handle);
    
    return true;
}



bool ConfigurationHelper::mergeConfig(const std::vector< std::string >& names, Configuration& result)
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
    result = entry->second;
    
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

        if(!result.merge(entry->second))
            return false;
    }    
    
//     std::cout << "Resulting config is :" << std::endl;
//     displayConfiguration(result);
    
    return true;
}

bool ConfigurationHelper::applyConfig(const std::string& configFilePath, RTT::TaskContext* context, const std::vector< std::string >& names)
{
    loadConfigFile(configFilePath);
    
    Configuration config;
    if(!mergeConfig(names, config))
    {
        throw std::runtime_error("Error, merging of configuarations for context " + context->getName() + " failed ");
        return false;
    }
    
    
    std::map<std::string, ConfigValue *>::const_iterator propIt;
    for(propIt = config.values.begin(); propIt != config.values.end(); propIt++)
    {
        if(!applyConfToProperty(context, propIt->first, *(propIt->second)))
        {
            std::cout << "ERROR configuration of " << propIt->first << " failed" << std::endl;
            throw std::runtime_error("ERROR configuration of "  + propIt->first + " failed for context " + context->getName());
            return false;
        }
    }

    return true;
}

bool ConfigurationHelper::applyConfig(RTT::TaskContext* context, const std::vector< std::string >& names)
{
    Bundle &bundle(Bundle::getInstance());
    
    //we need to figure out the model name first
    RTT::OperationInterfacePart *op = context->getOperation("getModelName");
    if(!op)
        throw std::runtime_error("Could not get model name of task");
    
    RTT::OperationCaller< ::std::string() >  caller(op);
    std::string modelName = caller();
    
    if(modelName.empty())
        throw std::runtime_error("ConfigurationHelper::applyConfig error, context did not give a valid model name (none at all)");
    
    return applyConfig(bundle.getConfigurationDirectory() + modelName + ".yml", context, names);
}

bool ConfigurationHelper::applyConfig(RTT::TaskContext* context, const std::string& conf1)
{
    std::vector<std::string> configs;
    configs.push_back(conf1);
    
    return applyConfig(context, configs);
}

bool ConfigurationHelper::applyConfig(RTT::TaskContext* context, const std::string& conf1, const std::string& conf2)
{
    std::vector<std::string> configs;
    configs.push_back(conf1);
    configs.push_back(conf2);
    
    return applyConfig(context, configs);
}

bool ConfigurationHelper::applyConfig(RTT::TaskContext* context, const std::string& conf1, const std::string& conf2, const std::string& conf3)
{
    std::vector<std::string> configs;
    configs.push_back(conf1);
    configs.push_back(conf2);
    configs.push_back(conf3);
    
    return applyConfig(context, configs);
}

bool ConfigurationHelper::applyConfig(RTT::TaskContext* context, const std::string& conf1, const std::string& conf2, const std::string& conf3, const std::string& conf4)
{
    std::vector<std::string> configs;
    configs.push_back(conf1);
    configs.push_back(conf2);
    configs.push_back(conf3);
    configs.push_back(conf4);
    
    return applyConfig(context, configs);
}

