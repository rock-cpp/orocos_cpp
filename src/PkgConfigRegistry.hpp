#pragma once
#include <map>
#include <vector>
#include <memory>
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

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
extern PkgConfigRegistryPtr __pkgcfgreg;

class PkgConfigRegistry
{
public:
    //!
    //! \brief Initialize PkgConfigRegistry
    //!
    //! Creates a singleton instance of the PkgConfigRegistry that can be
    //! retrieved with PkgConfigRegistry::get()
    //!
    //! PkgConfigRegistry searches the PkgConfig search path for oroGen or
    //! oroGenDeloyment Packages and loads them.
    //! This function works in two differnt modes: Either all relevant packages
    //! found in search path are loaded, or only a explicit selection of
    //! packages is loaded.
    //!
    //! The folders where to look for packages is defined by the PKG_CONFIG_PATH
    //! environment variable
    //!
    //! \param packageNames : Names of packages that should be searched and
    //!                       loaded
    //! \param load_all_packages : if \value true, all relevant packages found
    //!                            in search path are loaded. In this case,
    //!                            \var packageNames is ignored.
    //! \return Pointer to the PkgConfigRegistry. Can be \value nullptr, if
    //!         initialization failed
    static PkgConfigRegistryPtr initialize(const std::vector<std::string>& packageNames, bool loadAllPackages=false);
    //! Retrieve the singleton instance of PkgConfig, that was previously
    //! initialized with PkgConfigRegistry::initialize
    static PkgConfigRegistryPtr get();

    //! Avoid using this constructor and use the singleton intialization
    //! instead. \see PkgConfigRegistry::initialize
    PkgConfigRegistry(const std::vector<std::string>& packageNames, bool loadAllPackages=false);

    bool getDeployment(const std::string& name, PkgConfig& pkg, bool searchPackageIfNotLoaded=true);
    bool getTypekit(const std::string& name, TypekitPkgConfig &pkg, bool searchPackageIfNotLoaded=true);
    bool getOrogen(const std::string& name, OrogenPkgConfig& pkg, bool searchPackageIfNotLoaded=true);
    bool getOrocosRTT(PkgConfig& pkg, bool searchPackageIfNotLoaded=true);
    std::vector<std::string> getRegisteredDeploymentNames();
    std::vector<std::string> getRegisteredTypekitNames();
    std::vector<std::string> getRegisteredOrogenNames();

protected:
    //! To what kind of package a Pkgconfig file is related to is determined by
    //! its filename (NOT path, must be a file name!)
    bool isTransportPkg(const std::string& filename, std::string& typekitName, std::string& transportName, std::string &arch);
    bool isOrogenTasksPkg(const std::string& filename, std::string& orogenProjectName, std::string &arch);
    bool isOrogenProjectPkg(const std::string& filename, std::string& orogenProjectName);
    bool isTypekitPkg(const std::string& filename, std::string& typekitName, std::string &arch);
    bool isProxiesPkg(const std::string& filename, std::string& orogenProjectName);
    bool isDeploymentPkg(const std::string& filename, std::string& deploymentName);
    bool isOrocosRTTPkg(const std::string &filename, std::string &arch);

    //! [[deprecated(Due to performance issues with the matching of regular
    //!              expressions this function is now no longer used. Instead
    //!             'scan' calls 'loadOrogenPkg and loadDeploymentPkg)]]
    bool addFile(const std::string& filepath);
    bool loadOrogenPkg(const fs::path &filepath);
    bool loadDeploymentPkg(const fs::path &filepath);

    //! Search of a specific package identified by \p pname in \p searchPaths
    //! Tries to load oroGen, or deployment packages with related typekit and
    //! transports
    //! If \p searchPaths is an empty vector, it is determined from environment
    //! variable \see PkgConfigHelper::getSearchPathsFromEnvVar()
    bool findAndLoadPackage(const std::string& pname, std::vector<std::string> searchPaths = std::vector<std::string>());
    //! Load the packages defined in /p module_whitelist
    bool loadPackages(const std::vector<std::string> &packageNames, const std::vector<std::string> &searchPaths);
    //! Scans all folders in searchPath for orogen, and Deploment packages and
    //! loads them
    void loadAllPackages(const std::vector<std::string>& searchPaths);

    //! Containers to store PkgConfig files for different categories of
    //! libraries used in Rock
    std::map<std::string, PkgConfig> deployments;
    std::map<std::string, OrogenPkgConfig> orogen;
    std::map<std::string, TypekitPkgConfig> typekits;
    //! orocos-rtt library does not fit the other categories above
    PkgConfig orocosRTTPkg;
};

}
