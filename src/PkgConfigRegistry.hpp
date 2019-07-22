#include <map>
#include <vector>
#include <memory>

namespace orocos_cpp{
class PkgConfig
{
public:
    PkgConfig();
    /*!
     * \brief Get value of a variable defined within PKGConfig
     * Varibales in PKGConfig files can have arbitrary names. They are assigned by "varname=value".
     * Example:
     *   prefix=/my/prefix
     *   exec_prefix=${prefix}
     *   libdir=${prefix}/lib/orocos/types
     * \param variableName : name of the variable
     * \param value : assigned value of the variable \p variableName will be be retured here
     * \return true of value was successfully read. False if variable was not defined and thus value could net be determined.
     */
    bool getVariable(const std::string& variableName, std::string& value);

    /*!
     * \brief Get value of a property defined within PKGConfig
     * In PKGConfig files there exists a defined set of properties to describe a software module for compiling and linking. They are assigned by "propname: value".
     *   Example:
     *   Name: testTypekit
     *   Version: 0.0
     *   Requires: test
     *   Description: test types support for the Orocos type system
     *   Libs: -L${libdir} -ltest-typekit-gnulinux
     *   Cflags: -I${includedir} -I${includedir}/test/types "-DOROCOS_TARGET=gnulinux"
     * \param propertyName : name of the property
     * \param value : assigned value of the property \p propertyName will be be retured here
     * \return true of value was successfully read. False if varibale was not defined and thus value could net be determined.
     */
    bool getProperty(const std::string& propertyName, std::string& value);

    /*!
     * \brief load and parse a PKGConfig file
     * The file will be searched in paths specified in the PKG_CONFIG_PATH envirionment variable
     * \param filename
     */
    bool load(const std::string& filename);
    std::string name;
    std::string sourceFile;

    /*!
     * \brief Checks wether PKGConfig file has been loaded
     * \return
     */
    bool isLoaded();

protected:
    std::map<std::string, std::string> variables;
    std::map<std::string, std::string> properties;
};

class TypekitPkgConfig{
public:
    PkgConfig typekit;
    //! Maps transport name (e.g. 'mqueue') to the corresponding PKGConfig
    std::map<std::string, PkgConfig> transports;
};

class OrogenPkgConfig{
public:
    PkgConfig tasks;
    PkgConfig project;
    PkgConfig proxies;
};

class PkgConfigRegistry;
typedef std::shared_ptr<PkgConfigRegistry> PkgConfigRegistryPtr;
PkgConfigRegistryPtr __pkgcfgreg;

class PkgConfigRegistry
{
public:
    bool initialize();
    bool getDeployment(const std::string& name, PkgConfig& pkg);
    bool getTypekit(const std::string& name, TypekitPkgConfig &pkg);
    bool getOrogen(const std::string& name, OrogenPkgConfig& pkg);
    bool getOrocosRTT(PkgConfig& pkg);
    std::vector<std::string> getRegisteredDeploymentNames();
    std::vector<std::string> getRegisteredTypekitNames();
    std::vector<std::string> getRegisteredOrogenNames();
    static PkgConfigRegistryPtr get();

protected:
    //! To what kind of package a Pkgconfig file is related to is determined by its filename (NOT path, must be a file name!)
    bool isTransportPkg(const std::string& filename, std::string& typekitName, std::string& transportName, std::string &arch);
    bool isOrogenTasksPkg(const std::string& filename, std::string& orogenProjectName, std::string &arch);
    bool isOrogenProjectPkg(const std::string& filename, std::string& orogenProjectName);
    bool isTypekitPkg(const std::string& filename, std::string& typekitName, std::string &arch);
    bool isProxiesPkg(const std::string& filename, std::string& orogenProjectName);
    bool isDeploymentPkg(const std::string& filename, std::string& deploymentName);
    bool isOrocosRTTPkg(const std::string &filename, std::string &arch);

    bool addFile(const std::string& filepath);
    void scan(const std::vector<std::string>& searchPaths);

    std::map<std::string, PkgConfig> deployments;
    std::map<std::string, OrogenPkgConfig> orogen;
    std::map<std::string, TypekitPkgConfig> typekits;
};

}
