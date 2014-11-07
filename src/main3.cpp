#include "ConfigurationHelper.hpp"

int main(int argc, char**argv)
{
    ConfigurationHelper helper;
    helper.loadConfigFile("config.yml");
    
    std::vector<std::string> names;
    
    names.push_back("default");
    names.push_back("ground_based_sweeping");
//     names.push_back("foo");

    
    
    if(!helper.applyConfig(NULL, names))
        std::cout << "APPLY FAILED" << std::endl;
    
    return 0;
}