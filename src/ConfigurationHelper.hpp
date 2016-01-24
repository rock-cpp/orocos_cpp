#pragma once

#include <rtt/TaskContext.hpp>
#include <lib_config/Configuration.hpp>

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

private:
    std::map<std::string, libConfig::Configuration> subConfigs;
    bool mergeConfig(const std::vector<std::string> &names, libConfig::Configuration &result);
    bool applyConfToProperty(RTT::TaskContext* context, const std::string &propertyName, const libConfig::ConfigValue &value);
};

}//end of namespace

