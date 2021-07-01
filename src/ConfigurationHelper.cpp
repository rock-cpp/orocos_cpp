#include "ConfigurationHelper.hpp"
#include <rtt/types/TypeInfo.hpp>
#include <rtt/typelib/TypelibMarshaller.hpp>
#include <rtt/base/DataSourceBase.hpp>
#include <rtt/transports/corba/TaskContextProxy.hpp>
#include <rtt/types/TypekitRepository.hpp>

#include <boost/lexical_cast.hpp>
#include <rtt/OperationCaller.hpp>
#include <lib_config/Bundle.hpp>
#include <string>  
#include <limits>

#include "PluginHelper.hpp"
#include <lib_config/YAMLConfiguration.hpp>

using namespace orocos_cpp;
using namespace libConfig;



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
    
    // first, check if given config value is an int representation of the enum
    int32_t enum_int = -1;
    try {
        enum_int = std::stoi(conf.getValue());
    } 
    catch (std::invalid_argument& e1) {
        std::cerr << "Could not convert config value '" << conf.getValue() << "' to int." << std::endl;
    } 
    catch (std::out_of_range& e2) {
        std::cerr << "Could not convert config value '" << conf.getValue() << "' to int, because number is out of range of int." << std::endl;
    }

    std::string enumName = "";
    if (enum_int != -1) {
        //check if enum_int is valid  
        for (auto e : myenum->values()) {
            if (e.second == enum_int) {
                enumName = e.first;
                int32_t *val = static_cast<int32_t *>(value.getData());
                *val = enum_int;
                return true;
            }
        }

        if (enumName.empty()) {
            std::cerr << "Error : " << conf.getValue() << " is not a valid enum int representation " << std::endl;
            return false;
        }        
    }

    //values are sometimes given as RUBY constants. We need to remove the ':' in front of them
    
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


bool ConfigurationHelper::applyConfOnTyplibValue(Typelib::Value &value, const ConfigValue& conf)
{
    switch(value.getType().getCategory())
    {
        case Typelib::Type::Array:
        {
            const ArrayConfigValue *arrayConfig = dynamic_cast<const ArrayConfigValue *>(&conf);
            const Typelib::Array *array = dynamic_cast<const Typelib::Array *>(&value.getType());
            const Typelib::Type &indirect = array->getIndirection();
            
            size_t arraySize = array->getDimension();
            
            if(arrayConfig->getValues().size() != arraySize)
            {
                std::cout << "Error: Array " << arrayConfig->getName() << " of properties has different size than array in config file" << std::endl;
                return false;
            }
            
            for(size_t i = 0;i < arraySize; i++)
            {
                size_t offset =  indirect.getSize() * i;
                Typelib::Value v( reinterpret_cast<uint8_t *>(value.getData()) + offset , indirect);
                if(!applyConfOnTyplibValue(v, *(arrayConfig->getValues()[i])))
                    return false;
            }
        }
        break;
        case Typelib::Type::Compound:
        {
            const Typelib::Compound *comp = dynamic_cast<const Typelib::Compound *>(&value.getType());
            Typelib::Compound::FieldList::const_iterator it = comp->getFields().begin();
            const ComplexConfigValue *cpx = dynamic_cast<const ComplexConfigValue *>(&conf);
            auto confValues = cpx->getValues();
            
            for(;it != comp->getFields().end(); it++)
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
                std::cout << "is/are not members of " << comp->getName() << std::endl;
                return false;
            }
        }
            break;
        case Typelib::Type::Container:
            {
                const Typelib::Container *cont = dynamic_cast<const Typelib::Container *>(&(value.getType()));
                Typelib::zero(value);
                const Typelib::Type &indirect = cont->getIndirection();
                if(cont->kind() == "/std/string")
                {
                    if(conf.getType() != ConfigValue::SIMPLE)
                    {
                        std::cout << "Error, YAML representation << " << conf.getName() << " of type " << value.getType().getName() << " is not an array " << std::endl;
                        std::cout << "Error, got container in property, but config value is not a String " << std::endl;
                        return false;
                    }
                    const SimpleConfigValue *sconf = dynamic_cast<const SimpleConfigValue *>(&conf);

                    size_t chars = sconf->getValue().size();
                    for(size_t i = 0; i < chars; i++)
                    {
                        Typelib::Value singleChar((void *)( sconf->getValue().c_str() + i), indirect);
                        cont->push(value.getData(), singleChar);
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
                    const ArrayConfigValue *array = dynamic_cast<const ArrayConfigValue *>(&conf);

                    for(const std::shared_ptr<ConfigValue> val: array->getValues())
                    {
                        
                        //TODO check, this may be a memory leak
                        Typelib::Value v(new uint8_t[indirect.getSize()], indirect);
                        Typelib::init(v);
                        Typelib::zero(v);
                        
                        if(!applyConfOnTyplibValue(v, *(val)))
                        {
                            return false;
                        }
                        
                        cont->push(value.getData(), v);
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

bool ConfigurationHelper::applyConfToProperty(RTT::TaskContext* context, const std::string& propertyName, const libConfig::ConfigValue& value)
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

    //get data source
    RTT::base::DataSourceBase::shared_ptr ds = property->getDataSource();

    return applyConfigValueOnDSB(ds, typeInfo, value);

}


bool ConfigurationHelper::applyConfigValueOnDSB(RTT::base::DataSourceBase::shared_ptr dsb,
        const RTT::types::TypeInfo* typeInfo, const libConfig::ConfigValue& value){

    orogen_transports::TypelibMarshallerBase *typelibTransport =
            dynamic_cast<orogen_transports::TypelibMarshallerBase*>(
                    typeInfo->getProtocol(orogen_transports::TYPELIB_MARSHALLER_ID));

    //TODO make faster by adding getType to transport
    const Typelib::Type *type = typelibTransport->getRegistry().get(typelibTransport->getMarshallingType());

    orogen_transports::TypelibMarshallerBase::Handle *handle = typelibTransport->createSample();

    uint8_t *buffer = typelibTransport->getTypelibSample(handle);

    Typelib::Value dest(buffer, *type);

    if(typelibTransport->readDataSource(*dsb, handle))
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
    typelibTransport->writeDataSource(*dsb, handle);
    
    //destroy handle to avoid memory leak
    typelibTransport->deleteHandle(handle);
    
    return true;
}


bool ConfigurationHelper::applyConfig(RTT::TaskContext* context, const Configuration& config)
{     
    Configuration baseConf = config;
    
    if(overrides.find(context->getName()) != overrides.end())
    {
        Configuration &overrideConf = overrides.at(context->getName());
        if(!baseConf.merge(overrideConf))
            throw std::runtime_error("error: merging override config for task " + context->getName() + " failed");
    }
    std::map<std::string, std::shared_ptr<ConfigValue> >::const_iterator propIt;
    for(propIt = baseConf.getValues().begin(); propIt != baseConf.getValues().end(); propIt++)
    {
        if(!applyConfToProperty(context, propIt->first, *(propIt->second)))
        {
            std::cout << "ERROR configuration of " << propIt->first << " failed" << std::endl;
            throw std::runtime_error("ERROR: Apply configuration of variable '"  + propIt->first + "' failed for context " + context->getName());
            return false;
        }
    }

    return true;    
}


bool ConfigurationHelper::applyConfig(const std::string& configFilePath, RTT::TaskContext* context, const std::vector< std::string >& names)
{
    libConfig::MultiSectionConfiguration mcfg;
    mcfg.load(configFilePath);
    libConfig::Configuration config = mcfg.getConfig(names);
    
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
        try {
            context = RTT::corba::TaskContextProxy::Create(context->getName(), false);
            if(!context) throw std::runtime_error("Error, could not create Proxy for " + context->getName());
        } catch(...)
        {
            throw std::runtime_error("ConfigurationHelper::applyConfig: Error, could not create Proxy for " + context->getName());
        }
    }
    
    if(modelName.empty())
        throw std::runtime_error("ConfigurationHelper::applyConfig error, context did not give a valid model name (none at all)");
    
    libConfig::Configuration retConfig = bundle.taskConfigurations.getConfig(
                modelName, names);
    bool ret = applyConfig(context, retConfig);
    
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

bool ConfigurationHelper::registerOverride(const std::string& taskName, Configuration& config)
{
    if(overrides.find(taskName) == overrides.end())
    {
        overrides.insert(std::make_pair(taskName, config));
        std::cout << "adding config override for task " << taskName << std::endl;
        config.print();
        return true;
    }
    
    return false;
}

YAML::Emitter &toYAML(YAML::Emitter &out, const Typelib::Numeric &type, const Typelib::Value &value){

    switch(type.getNumericCategory())
    {
    case Typelib::Numeric::Float:
        if(type.getSize() == sizeof(float))
        {
            out << *(static_cast<float *>(value.getData()));
        }
        else
        {
            out << *(static_cast<double *>(value.getData()));
        }
        break;
    case Typelib::Numeric::SInt:
        switch(type.getSize())
        {
        case sizeof(int8_t):
            out << *(static_cast<int8_t *>(value.getData()));
            break;
        case sizeof(int16_t):
            out << *(static_cast<int16_t *>(value.getData()));
            break;
        case sizeof(int32_t):
            out << *(static_cast<int32_t *>(value.getData()));
            break;
        case sizeof(int64_t):
            out << *(static_cast<int64_t *>(value.getData()));
            break;
        default:
            std::cerr << "Error, got integer of unexpected size " << type.getSize() << std::endl;
            throw std::runtime_error("got integer of unexpected size " + type.getSize());
            break;
        }
        break;
    case Typelib::Numeric::UInt:
    {
        switch(type.getSize())
        {
        case sizeof(uint8_t):
            out << *(static_cast<uint8_t *>(value.getData()));
            break;
        case sizeof(uint16_t):
            out << *(static_cast<uint16_t *>(value.getData()));
            break;
        case sizeof(uint32_t):
            out << *(static_cast<uint32_t *>(value.getData()));
            break;
        case sizeof(uint64_t):
            out << *(static_cast<uint64_t *>(value.getData()));
            break;
        default:
            std::cout << "Error, got integer of unexpected size " << type.getSize() << std::endl;
            throw std::runtime_error("got integer of unexpected size " + type.getSize());
            break;
        }
    }
        break;
    case Typelib::Numeric::NumberOfValidCategories:
        throw std::runtime_error("Internal Error: Got invalid Category");
        break;
    }

    return out;
}

YAML::Emitter &toYAML(YAML::Emitter &out, const Typelib::Container &type, const Typelib::Value &value){

    const size_t size = type.getElementCount(value.getData());

    if(type.kind() == "/std/string")
    {
        const std::string *content = static_cast<const std::string *>(value.getData());

        out << *content;
        return out;
    }

    //std::vector
    out << YAML::BeginSeq;
    for(size_t i = 0; i < size; i++)
    {
        Typelib::Value elem = type.getElement(value.getData(), i);
        out << elem;
    }
    out << YAML::EndSeq;

    return out;
}

YAML::Emitter &toYAML(YAML::Emitter &out, const Typelib::Compound &type, const Typelib::Value &value){
    out << YAML::BeginMap;

    uint8_t *data = static_cast<uint8_t *>( value.getData());

    Typelib::Compound::FieldList const fields = type.getFields();
    for(const Typelib::Field &field: fields)
    {
        Typelib::Value fieldV(data + field.getOffset(), field.getType());
        out << YAML::Key << field.getName();
        out << YAML::Value << fieldV;
    }

    out << YAML::EndMap;

    return out;
}

YAML::Emitter &toYAML(YAML::Emitter &out, const Typelib::Array &type, const Typelib::Value &value){
    out << YAML::Flow;
    out << YAML::BeginSeq;

    const Typelib::Type &indirect(type.getIndirection());
    for(size_t i = 0; i < type.getDimension(); i++)
    {
        Typelib::Value arrayV(static_cast<uint8_t *>(value.getData()) + i * indirect.getSize(), indirect);
        out << arrayV;
    }
    out << YAML::EndSeq;
    return out;
}

YAML::Emitter &toYAML(YAML::Emitter &out, const Typelib::Enum &type, const Typelib::Value &value){
    Typelib::Enum::integral_type *intVal = (static_cast<Typelib::Enum::integral_type *>(value.getData()));
    out << type.get(*intVal);
    return out;
}

//Duplicates logic from std::shared_ptr< ConfigValue > TypelibConfiguration::getFromValue(Typelib::Value& value)
//Problem with the ConfigValue type is, that it does not contain any details
//about the SimpleType, i.e. if it's a float, string, int, bool, enum.
//This type information is present in Typelib::Value, bot no more in ConfigValue
//so theres no way for proper YAML serialization of ConfigValues
YAML::Emitter &operator <<(YAML::Emitter &out, const Typelib::Value &value)
{
    const Typelib::Type& type = value.getType();
    if(type.getCategory() == Typelib::Type::Array){
        const Typelib::Array &array = static_cast<const Typelib::Array &>(value.getType());
        toYAML(out, array, value);
    }
    else if(type.getCategory() == Typelib::Type::Compound){
        const Typelib::Compound &compound = static_cast<const Typelib::Compound &>(value.getType());
        toYAML(out, compound, value);
    }
    else if(type.getCategory() == Typelib::Type::Container){
        const Typelib::Container &container = static_cast<const Typelib::Container &>(value.getType());
        toYAML(out, container, value);
    }
    else if(type.getCategory() == Typelib::Type::Enum){
        const Typelib::Enum &en = static_cast<const Typelib::Enum &>(value.getType());
        toYAML(out, en, value);
    }
    else if(type.getCategory() == Typelib::Type::Numeric){
        const Typelib::Numeric &num = static_cast<const Typelib::Numeric &>(value.getType());
        toYAML(out, num, value);
    }
    else if(type.getCategory() == Typelib::Type::NullType){
        throw std::runtime_error("Got Unsupported Category: NullType");
    }
    else if(type.getCategory() == Typelib::Type::Opaque){
        throw std::runtime_error("Got Unsupported Category: Opaque");
    }
    else if(type.getCategory() == Typelib::Type::Pointer){
        throw std::runtime_error("Got Unsupported Category: Pointer");
    }
    else{
        throw std::runtime_error(std::string("Got Unexpected Category: ")+std::to_string(type.getCategory()));
    }
    return out;
}


std::string ConfigurationHelper::getYamlString(const Typelib::Value &value)
{
    YAML::Emitter out;

    out << value;
    return out.c_str();
}

