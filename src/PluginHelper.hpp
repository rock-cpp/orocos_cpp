#ifndef PLUGINHELPER_H
#define PLUGINHELPER_H

#include <vector>
#include <string>

class PluginHelper
{
public:
    static void loadAllPluginsInDir(const std::string &path);

    /**
     * This function loads the typekits and transports of the given
     * component.
     * */
    static bool loadTypekitAndTransports(const std::string &componentName);

    /**
     * This function parses the local pkg_config file to
     * figure out which typkits are need by the given component.
     * 
     * @return A vector containing the names of the needed typekits
     * */
    static std::vector<std::string> getNeededTypekits(const std::string &componentName);
};

#endif // PLUGINHELPER_H
