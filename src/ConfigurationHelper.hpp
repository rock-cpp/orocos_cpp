#ifndef CONFIGURATIONHELPER_H
#define CONFIGURATIONHELPER_H

#include <rtt/TaskContext.hpp>


class ConfigValue
{
public:
    enum Type {
        SIMPLE,
        COMPLEX,
        ARRAY,
    };
    
    enum Type type;
    std::string name;
    
    virtual bool merge(const ConfigValue *other) = 0;
    
protected:
    ConfigValue(enum Type);
    virtual ~ConfigValue();
};

class SimpleConfigValue : public ConfigValue
{
public:
    virtual bool merge(const ConfigValue* other);
    SimpleConfigValue();
    std::string value;
};

class ComplexConfigValue : public ConfigValue
{
public:
    virtual bool merge(const ConfigValue* other);
    ComplexConfigValue();
    std::map<std::string, ConfigValue *> values;
};

class ArrayConfigValue : public ConfigValue
{
public:
    virtual bool merge(const ConfigValue* other);
    ArrayConfigValue();
    std::vector<ConfigValue *> values;
};

class Configuration
{
public:
    std::string name;
    bool merge(const Configuration &other);
    std::map<std::string, ConfigValue *> values;
};

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
    bool parseStringBuffer(Configuration& curConfig, const std::string& buffer);
};

#endif // CONFIGURATIONHELPER_H
