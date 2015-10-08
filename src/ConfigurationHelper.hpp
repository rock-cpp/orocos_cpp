#ifndef CONFIGURATIONHELPER_H
#define CONFIGURATIONHELPER_H

#include <rtt/TaskContext.hpp>
#include "Configuration.hpp"

namespace orocos_cpp
{

class ConfigurationHelper
{
    std::map<std::string, Configuration> subConfigs;
public:

    bool applyConfig(const std::string &configFilePath, RTT::TaskContext *context, const std::vector<std::string> &names);
    bool applyConfig(RTT::TaskContext *context, const std::vector<std::string> &names);
    bool applyConfig(RTT::TaskContext *context, const std::string &conf1);
    bool applyConfig(RTT::TaskContext *context, const std::string &conf1, const std::string &conf2);
    bool applyConfig(RTT::TaskContext *context, const std::string &conf1, const std::string &conf2, const std::string &conf3);
    bool applyConfig(RTT::TaskContext *context, const std::string &conf1, const std::string &conf2, const std::string &conf3, const std::string &conf4);
    /**
     * This method applies a configuration to a given task context by a given YAML String.
     *
     * @param context The task context for which the configuration should be applied.
     * @configYamlString This string has to contain a valid YAML string describing the configuration that should be applied to the task.
     * @returns boolean value, true on success, false on fail.
     */
    bool applyConfigString(RTT::TaskContext *context, const std::string &configYamlString);

    /**
     * This function analyses the given string and replaces variables.
     * The variables in the string taht will be evaluated are given by "<%= ENV('xyz') %>" or
     * "<%= BUNDLES('xyz') %>". In the case of ENV, the given value xyz will be treeted as an
     * environment variable that will be replaced by its current value. BUNDLES specifies that xyz in that case is a
     * relative path (relative with respect to a bundle root). This path will be converted in an absolute path
     * by adding the prefix path of the bundle in which the path was found (starting with the default bundle if specified).
     *
     * @param val Is a const string reference parameter, which contains the original string with the variables.
     * @returns String containing the enhanced string with all replacements.
     */
    static std::string applyStringVariableInsertions(const std::string &val);
private:
    /**
     * Finnaly apply the config values.
     * \param conf The configuration to be set,
     * \param context to the given context.
     * \return True on success otherwise false.
     */
    bool setConfig(const Configuration &conf, RTT::TaskContext *context);
    bool loadConfigFile(const std::string &path);
    bool mergeConfig(const std::vector<std::string> &names, Configuration &result);
    bool applyConfToProperty(RTT::TaskContext* context, const std::string &propertyName, const ConfigValue &value);
    bool parseStringBuffer(Configuration &curConfig, const std::string &buffer);
};

}//end of namespace

#endif // CONFIGURATIONHELPER_H
