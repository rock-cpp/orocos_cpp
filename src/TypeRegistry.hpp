#pragma once

#include <string>
#include <map>
#include <memory>
#include "PkgConfigRegistry.hpp"


namespace Typelib
{
    class Registry;
}

namespace orocos_cpp
{

class TypeRegistry;
typedef std::shared_ptr<TypeRegistry> TypeRegistryPtr;

class TypeRegistry
{
protected:
    std::map<std::string, std::string> typeToTypekit;
    std::map<std::string, unsigned> taskStateToID;
    PkgConfigRegistryPtr pkgreg;

public:
    TypeRegistry(PkgConfigRegistryPtr pkgreg);
    TypeRegistry();
    
    /**
      * Load single TypeRegistry for a given typekit name
      */
    bool loadTypeRegistry(const std::string& typekitName);
    /**
     * Loads type registryies for all typekits defined in \var pkgreg
     */
    bool loadTypeRegistries();
    
    bool getTypekitDefiningType(const std::string &typeName, std::string &typekitName);

    /**
     * Returns the ID of a given state name of a task.
     * If the task model was not added to the TypeRegistry, this function
     * trigger the loading process.
     * @param task_model_name e.g. "auv_control::AccelerationController"
     * @param state_name e.g. "CONTROLLING"
     * @param id the corresponding state ID
     *
     * @returns false if the state or the task is unknown and cant be loaded
     */
    bool getStateID(const std::string &task_model_name, const std::string &state_name, unsigned& id);

protected:
    /**
     * Loads a tlb file to the registry.
     * @param path the path to the tlb file
     */
    bool loadRegistry(const std::string &path, Typelib::Registry* registry);

    /**
     * Saves the stateToIDMapping from a given tlb file.
     * @param path the path to the tlb file
     */
    bool loadStateToIDMapping(const std::string &path);

    /**
     * Saves the typeToTypekitMapping from a given typelist file.
     * @param path the path to the typelist file
     * @param typekitName the name of the corresponding typekit
     */
    bool loadTypeToTypekitMapping(const std::string &path, const std::string &typekitName);
};
}
