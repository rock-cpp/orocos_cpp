#define once

#include <string>
#include <map>

namespace Typelib
{
    class Registry;
}

namespace orocos_cpp
{

class TypeRegistry
{
    std::map<std::string, std::string> typeToTypekit;
    std::map<std::string, unsigned> taskStateToID;
public:
    TypeRegistry();
    
    /**
     * Laods all type registries in order to receive the nessesary informations.
     */
    bool loadTypeRegistries();
    
    bool getTypekitDefiningType(const std::string &typeName, std::string &typekitName);

    /**
     * Returns the ID of a given state name of a task.
     * @param task_model_name e.g. "auv_control::AccelerationController"
     * @param state_name e.g. "CONTROLLING"
     * @param id the corresponding state ID
     *
     * @returns false if the state or the task is unknown
     */
    bool getStateID(const std::string &task_model_name, std::string &state_name, unsigned& id) const;

protected:
    /**
     * Loads the registry of a given task model name.
     */
    bool loadRegistry(const std::string &task_model_name, Typelib::Registry* registry);
};

}
