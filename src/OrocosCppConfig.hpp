#pragma once
#include <vector>
#include <string>

namespace orocos_cpp {
class OrocosCppConfig{
public:
    OrocosCppConfig() :
        package_initialization_whitelist(std::vector<std::string>()),
        load_all_packages(false),
        init_bundle(false),
        load_task_configs(false),
        init_type_registry(false),
        load_typekits(true),
        corba_host(""),
        init_corba(true)
    {}
    //! Specifies the names of oroGen packages that should be loaded at
    //! initialization time.
    //! Loading of modules includes IO operations on the hard disk. For
    //! real-time systems such operations should be avoided at runtime.
    //! It is suggested to pre-load the needed modules at initialization time
    //! instead.
    std::vector<std::string> package_initialization_whitelist;
    //! Indicate whether all installed Rock-packages should be installed
    //! If set to true, \var package_initialization_whitelist will be ignored
    //! and all isntalled pacakges will be loaded on start-up.
    bool load_all_packages;

    //! should the currently selected bundle be initialized?
    //! If set to \value true, the ROCK_BUNDLE and ROCK_BUNDLE_PATH evironment
    //! variables are evaluated to determine selected bundle that should be
    //! used:
    //!    * ROCK_BUNDLE: defines the selected bundle.
    //!    * ROCK_BUNDLE_PATH: Defines the search path where to look for
    //!      bundles
    //! If set to \value false, configuration files in the bundle will not be
    //! avaliable.
    bool init_bundle;

    //! Should task configuration configurations in bundle be loaded?
    //! Only evaluated if \var init_bundle is \value true.
    bool load_task_configs;

    //! Should type-registry be initialized
    //! If set to \value true, the TypeRegistry will be initialized with the
    //! typelib packages listed in \var module_initialization_whitelist.
    //!
    //! The type registry allows to convert between numerice and symbolic ENUM
    //! values and therefore need to parse the typekits tlb files.
    bool init_type_registry;

    //! Should typekits be initialized
    //! If set to \value true, the Typekits for the
    //! packages listed in \var module_initialization_whitelist will be
    //! loaded.
    bool load_typekits;

    //! Hostname or IP of the CORBA Nameservice to connect to
    //! If an empty string is given, the locally running CORBA nameservice
    //! will be used.
    std::string corba_host;

    //! Should CORBA be initialized.
    //! If set to false no interaction with TaskContexts can be carreid out!
    bool init_corba;
};
}
