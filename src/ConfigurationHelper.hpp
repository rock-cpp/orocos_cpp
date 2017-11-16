#pragma once

#include <rtt/TaskContext.hpp>
#include <lib_config/Configuration.hpp>


//forwards:

namespace Typelib{
    class Value;
}

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

private:
    std::map<std::string, libConfig::Configuration> subConfigs;
    std::map<std::string, libConfig::Configuration> overrides;
    bool mergeConfig(const std::vector<std::string> &names, libConfig::Configuration &result);
    bool applyConfToProperty(RTT::TaskContext* context, const std::string &propertyName, const libConfig::ConfigValue &value);
};

}//end of namespace

