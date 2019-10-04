#pragma once
#include "PkgConfigRegistry.hpp"
#include "TypeRegistry.hpp"
#include <lib_config/Bundle.hpp>
#include "OrocosCppConfig.hpp"


namespace orocos_cpp {

//Harmonize namespace for simpler API usage
typedef libConfig::SingleBundle SingleBundle;
typedef std::shared_ptr<SingleBundle> SingleBundlePtr;
typedef libConfig::Bundle Bundle;
typedef std::shared_ptr<Bundle> BundlePtr;

class OrocosCpp{
public:
    bool initialize(const OrocosCppConfig& config);
    PkgConfigRegistryPtr package_registry;
    TypeRegistryPtr type_registry;
    BundlePtr bundle;
};
}
