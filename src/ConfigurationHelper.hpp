#pragma once

#include <rtt/TaskContext.hpp>
#include <lib_config/Configuration.hpp>
#include <yaml-cpp/yaml.h>
#include <typelib/typemodel.hh>
#include <typelib/value.hh>
#include <lib_config/YAMLConfiguration.hpp>


//forwards:

namespace Typelib{
    class Value;
}

YAML::Emitter &operator <<(YAML::Emitter &out, const Typelib::Value &value);

namespace orocos_cpp
{


class ConfigurationHelper
{
public:

    /**
     * Applies the given configuration to the task.
     * \param conf The configuration to be set,
     * \param context to the given context.
     * \return True on success otherwise false.
     */
    bool applyConfig(RTT::TaskContext *context, const libConfig::Configuration &config);
    bool applyConfig(const std::string &configFilePath, RTT::TaskContext *context, const std::vector<std::string> &names);
    bool applyConfig(RTT::TaskContext *context, const std::vector<std::string> &names);
    bool applyConfig(RTT::TaskContext *context, const std::string &conf1);
    bool applyConfig(RTT::TaskContext *context, const std::string &conf1, const std::string &conf2);
    bool applyConfig(RTT::TaskContext *context, const std::string &conf1, const std::string &conf2, const std::string &conf3);
    bool applyConfig(RTT::TaskContext *context, const std::string &conf1, const std::string &conf2, const std::string &conf3, const std::string &conf4);
    bool registerOverride(const std::string& taskName, libConfig::Configuration &config);
    
    /**
     * @brief Function applying configuration value on a DataSourceBase object.
     * @param dsb The shared pointer object pointing to the DataSourceBase object. This will be modified!
     * @param typeInfo TypeInfo object containing the type information.
     * @param value Is the ConfigValue object which will be applied.
     * @return True if it runs through (probably correct), false if an error was detected.
     */
    bool applyConfigValueOnDSB(RTT::base::DataSourceBase::shared_ptr dsb,
            const RTT::types::TypeInfo* typeInfo, const libConfig::ConfigValue& value);
    bool applyConfOnTyplibValue(Typelib::Value &value, const libConfig::ConfigValue& conf);
    /**
     * @brief Convinience functions to load data samples from YAML
     * T value should be serializable data type such as base::samples::RigidBodyState_m (not opaque types)
     */
    template<typename T>
    bool loadTypeFromYaml(T &result, const libConfig::ConfigValue& conf, const Typelib::Type& typemodel){
        Typelib::Value value((void*)&result, typemodel);
        return applyConfOnTyplibValue(value, conf);
    }
    template<typename T>
    bool loadTypeFromYaml(T &result, const YAML::Node& node, const Typelib::Type& typemodel){
        libConfig::YAMLConfigParser parser;
        std::shared_ptr<libConfig::ConfigValue> conf =  parser.getConfigValue(node);
        return loadTypeFromYaml(result, *conf, typemodel);
    }
    template<typename T>
    bool loadTypeFromYaml(T &result, const std::string& yamlstring, const Typelib::Type& typemodel){
        YAML::Node docs = YAML::Load(yamlstring);
        return loadTypeFromYaml(result, docs, typemodel);
    }
    bool applyConfToProperty(RTT::TaskContext* context, const std::string &propertyName, const libConfig::ConfigValue &value);


    std::string getYamlString(const Typelib::Value &value);

private:
    std::map<std::string, libConfig::Configuration> overrides;
};


}//end of namespace


