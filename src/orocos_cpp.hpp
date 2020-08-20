#pragma once
#include "PkgConfigRegistry.hpp"
#include "TypeRegistry.hpp"
#include <lib_config/Bundle.hpp>
#include "OrocosCppConfig.hpp"
#include <rtt/transports/corba/TaskContextProxy.hpp>


#define DEFAULT_OROCOS_MAX_MESSAGE_SIZE 1000000000
namespace orocos_cpp {

//Harmonize namespace for simpler API usage
typedef libConfig::SingleBundle SingleBundle;
typedef std::shared_ptr<SingleBundle> SingleBundlePtr;
typedef libConfig::Bundle Bundle;
typedef std::shared_ptr<Bundle> BundlePtr;

class OrocosCpp{
public:
    bool initialize(const OrocosCppConfig& config, bool quiet=true);
    RTT::corba::TaskContextProxy* getTaskContext(std::string name);
    /*!
     * Loads all typkits required for a task model or from a package
     * All typekits were loaded to properly create a TaskContextProxy for an
     * task of the given model type.
     * This includes the load of all typekits that are directly required and all
     * depended requirements.
     * @param packageOrTaskModelName The Name of a task model, e.g., "camera_usb::Task" or package (e.g. "camera_usb")
     * @return Returns True if new task models were loaded. Returns False if no
     *         model was loaded (also if no load was required)! So the return
     *         value is no indicator for ther presence of the typekit. If there
     *         was an error during loading, std::runtime_error will be thrown.
     *         Thus, the termination of the function can be understood as that
     *         the typekit is present, regardless of the return value.
     * @throws std::runtime_error if errors during loading of a typekit occur
     */
    bool loadAllTypekitsForModel(std::string packageOrTaskModelName);
    static std::string applyStringVariableInsertions(const std::string& cnd_yaml);

    PkgConfigRegistryPtr package_registry;
    TypeRegistryPtr type_registry;
    BundlePtr bundle;
};
}
