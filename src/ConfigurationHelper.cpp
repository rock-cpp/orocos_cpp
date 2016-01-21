#include "ConfigurationHelper.hpp"
#include <rtt/types/TypeInfo.hpp>
#include <rtt/typelib/TypelibMarshaller.hpp>
#include <rtt/base/DataSourceBase.hpp>
#include <rtt/transports/corba/TaskContextProxy.hpp>
#include <rtt/types/TypekitRepository.hpp>

#include <boost/lexical_cast.hpp>
#include <rtt/OperationCaller.hpp>
#include "Bundle.hpp"
#include <string>  
#include <limits>

#include "PluginHelper.hpp"
#include "YAMLConfiguration.hpp"

using namespace orocos_cpp;



template <typename T>
bool applyValue(Typelib::Value &value, const SimpleConfigValue& conf)
{
    T *val = static_cast<T *>(value.getData());
    try {
        *val = boost::lexical_cast<T>(conf.getValue());
    } catch (boost::bad_lexical_cast bc)
    {
        std::cout << "Error, could not set value " << conf.getValue() << " on property " << conf.getName() << " Bad lexical cast : " << bc.what() << std::endl;
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
        *val = boost::numeric_cast<uint8_t>(boost::lexical_cast<unsigned int>(conf.getValue()));
    } catch (boost::bad_lexical_cast bc)
    {
        std::cout << "Error, could not set value " << conf.getValue() << " on property " << conf.getName() << " Bad lexical cast : " << bc.what() << std::endl;
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
        *val = boost::numeric_cast<int8_t>(boost::lexical_cast<int>(conf.getValue()));
    } catch (boost::bad_lexical_cast bc)
    {
        std::cout << "Error, could not set value " << conf.getValue() << " on property " << conf.getName() << " Bad lexical cast : " << bc.what() << std::endl;
        std::cout << " Target Type " << value.getType().getName() << std::endl;
        return false;
    }
    return true;
}

template <>
bool applyValue<double>(Typelib::Value &value, const SimpleConfigValue& conf)
{
    double *val = static_cast<double *>(value.getData());
    std::string  copy = conf.getValue();
    std::transform(copy.begin(), copy.end(), copy.begin(), ::tolower);
    if (copy.find("nan") != std::string::npos) {
	*val = std::numeric_limits<double>::quiet_NaN();
    } 
    else 
    {
      try {
	  *val = boost::lexical_cast<double>(conf.getValue());
      } 
      catch (boost::bad_lexical_cast bc)
      {
	  std::cout << "Error, could not set value " << conf.getValue() << " on property " << conf.getName() << " Bad lexical cast : " << bc.what() << std::endl;
	  std::cout << " Target Type " << value.getType().getName() << std::endl;
	  return false;
      }
    }
    return true;
}

bool applyConfOnTypelibEnum(Typelib::Value &value, const SimpleConfigValue& conf)
{
    const Typelib::Enum *myenum = dynamic_cast<const Typelib::Enum *>(&(value.getType()));

    if(conf.getValue().empty())
    {
        std::cout << "Error, given enum is an empty string" << std::endl;
        return false;
    }
    
    //values are sometimes given as RUBY constants. We need to remove the ':' in front of them
    std::string enumName;
    if(conf.getValue().at(0) == ':')
        enumName = conf.getValue().substr(1, conf.getValue().size());
    else
        enumName = conf.getValue();
    
    std::map<std::string, int>::const_iterator it = myenum->values().find(enumName);
    
    if(it == myenum->values().end())
    {
        std::cout << "Error : " << conf.getValue() << " is not a valid enum name " << std::endl;
        std::cout << "Valid enum names :" << std::endl;
        for(const std::pair<std::string, int> &v : myenum->values())
        {
            if(!v.first.empty())
            {
                if(v.first.at(0) == ':')
                    enumName = v.first.substr(1, v.first.size());
                else
                    enumName = v.first;

                std::cout << enumName << std::endl;
            }
        }
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
        {
            //HACK typelib encodes bools as unsigned integer. Brrrrr
            std::string lowerCase = conf.getValue();
            std::transform(lowerCase.begin(), lowerCase.end(), lowerCase.begin(), ::tolower);
            if(lowerCase == "true")
            {
                return applyConfOnTypelibNumeric(value, SimpleConfigValue("1"));
            }
            if(lowerCase == "false")
            {
                return applyConfOnTypelibNumeric(value, SimpleConfigValue("0"));
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
        }
            break;
        case Typelib::Numeric::NumberOfValidCategories:
            throw std::runtime_error("Internal Error: Got invalid Category");
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
            
            if(arrayConfig.getValues().size() != arraySize)
            {
                std::cout << "Error: Array " << arrayConfig.getName() << " of properties has different size than array in config file" << std::endl;
                return false;
            }
            
            for(size_t i = 0;i < arraySize; i++)
            {
                size_t offset =  indirect.getSize() * i;
                Typelib::Value v( reinterpret_cast<uint8_t *>(value.getData()) + offset , indirect);
                if(!applyConfOnTyplibValue(v, *(arrayConfig.getValues()[i])))
                    return false;
            }
        }
        break;
        case Typelib::Type::Compound:
        {
            const Typelib::Compound &comp = dynamic_cast<const Typelib::Compound &>(value.getType());
            Typelib::Compound::FieldList::const_iterator it = comp.getFields().begin();
            const ComplexConfigValue &cpx = dynamic_cast<const ComplexConfigValue &>(conf);
            auto confValues = cpx.getValues();
            
            for(;it != comp.getFields().end(); it++)
            {
                std::map<std::string, std::shared_ptr<ConfigValue> >::const_iterator confIt = confValues.find(it->getName());
                Typelib::Value fieldValue(((uint8_t *) value.getData()) + it->getOffset(), it->getType());
                if(confIt == confValues.end())
                {
                    //even if we don't configure this one, we still need to initialize it
//                     initTyplibValueRecusive(fieldValue);
                    continue;
                }

                std::shared_ptr<ConfigValue> curConf = confIt->second;
                
                confValues.erase(confIt);
                
                if(!applyConfOnTyplibValue(fieldValue, *curConf))
                    return false;
            }
            if(!confValues.empty())
            {
                std::cout << "Error :" << std::endl;
                for(std::map<std::string, std::shared_ptr<ConfigValue> >::const_iterator it = confValues.begin(); it != confValues.end();it++)
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
                Typelib::zero(value);
                const Typelib::Type &indirect = cont.getIndirection();
                if(cont.kind() == "/std/string")
                {
                    const SimpleConfigValue &sconf = dynamic_cast<const SimpleConfigValue &>(conf);

                    size_t chars = sconf.getValue().size();
                    for(size_t i = 0; i < chars; i++)
                    {
                    	Typelib::Value singleChar((void *)( sconf.getValue().c_str() + i), indirect);
                        cont.push(value.getData(), singleChar);
                    }
                    break;
                }
                else
                {
                    if(conf.getType() != ConfigValue::ARRAY)
                    {
                        std::cout << "Error, YAML representation << " << conf.getName() << " of type " << value.getType().getName() << " is not an array " << std::endl;
                        std::cout << "Error, got container in property, but config value is not an array " << std::endl;
                        return false;
                    }
                    const ArrayConfigValue &array = dynamic_cast<const ArrayConfigValue &>(conf);
                    

                    for(const std::shared_ptr<ConfigValue> val: array.getValues())
                    {
                        
                        //TODO check, this may be a memory leak
                        Typelib::Value v(new uint8_t[indirect.getSize()], indirect);
                        Typelib::init(v);
                        Typelib::zero(v);
                        
                        if(!applyConfOnTyplibValue(v, *(val)))
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

bool ConfigurationHelper::applyConfig(RTT::TaskContext* context, const Configuration& config)
{
    std::map<std::string, std::shared_ptr<ConfigValue> >::const_iterator propIt;
    for(propIt = config.getValues().begin(); propIt != config.getValues().end(); propIt++)
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


bool ConfigurationHelper::applyConfig(const std::string& configFilePath, RTT::TaskContext* context, const std::vector< std::string >& names)
{
    YAMLConfigParser parser;
    parser.loadConfigFile(configFilePath, subConfigs);
    
    Configuration config("Merged");
    if(!mergeConfig(names, config))
    {
        throw std::runtime_error("Error, merging of configuarations for context " + context->getName() + " failed ");
        return false;
    }
    
    //finally apply:
    return applyConfig(context, config);

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

    bool syncNeeded = PluginHelper::loadAllTypekitsForModel(modelName);
    
    //this is not a prox, we don't need to sync
    if(!dynamic_cast<RTT::corba::TaskContextProxy *>(context))
    {
        syncNeeded = false;
    }
    
    if(syncNeeded)
    {
        context = RTT::corba::TaskContextProxy::Create(context->getName(), false);
    }
    
    if(modelName.empty())
        throw std::runtime_error("ConfigurationHelper::applyConfig error, context did not give a valid model name (none at all)");
    
    bool ret = applyConfig(bundle.getConfigurationDirectory() + modelName + ".yml", context, names);
    
    if(syncNeeded)
        delete context;
        
    return ret;
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
