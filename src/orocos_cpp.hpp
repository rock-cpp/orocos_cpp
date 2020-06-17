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
    inline bool loadAllTypekitsForModel(std::string packageOrTaskModelName);
    static std::string applyStringVariableInsertions(const std::string& cnd_yaml);

    PkgConfigRegistryPtr package_registry;
    TypeRegistryPtr type_registry;
    BundlePtr bundle;
};
}
