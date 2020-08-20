#pragma once

#include <string>
#include <map>
#include <memory>
#include "PkgConfigRegistry.hpp"
#include <typelib/typemodel.hh>


namespace Typelib
{
    class Registry;
}

namespace orocos_cpp
{

class TypeRegistry;
typedef std::shared_ptr<TypeRegistry> TypeRegistryPtr;

/*!
 * \brief The TypeRegistry class goves access to content from TLB files
 *
 * Note that it does not handle the loading of transports.
 */
class TypeRegistry
{
protected:
    std::map<std::string, std::string> typeToTypekit;
    std::map<std::string, unsigned> taskStateToID;
    PkgConfigRegistryPtr pkgreg;

public:
    TypeRegistry(PkgConfigRegistryPtr pkgreg);
    TypeRegistry();
    
    /*!
     * \brief Load the registry for a typekit identified by its name
     *
     * The function keeps track of all registries that have been loade successfully
     * and only loads them, if they have not been laoded before, or \p force
     * is set to true.
     *
     * Typekits contain all types that are required from a orogen Packages,
     * including this that originate from other typekits. Thus it's not needed
     * to explicitly load all imported typekits explicitly.
     *
     * \param typekitName : Typekit to load
     * \param force : Forcefully load the Typekit registry, even if it was loaded before
     * \return True if typekit registry was successfully loaded, or was already loaded before
     */
    bool loadTypeRegistry(const std::string& typekitName, bool force=false);

    /*!
     * \brief Load all Typekits that are registered in \p pkgreg
     *
     * Note that this function does not load all typekit that are presetn on the
     * system, but only those which have been loaded in the PkgConfigRegistry.
     * To load all typekits, pass a PkgConfigRegistry, where all pakcages have
     * been loaded.
     *
     * \return true if all Typekits have been loaded successfully
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
    bool hasType(const std::string& typeName);
    const Typelib::Type *getTypeModel(const std::string& typeName);
    std::shared_ptr<Typelib::Registry> registry;
    std::vector<std::string> loadedTypekits;

protected:
    /**
     * Loads a tlb file into meber varibale
     * std::shared_ptr<Typelib::Registry> registry
     * @param path the path to the tlb file
     */
    bool loadTypelibRegistry(const std::string &path);

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
