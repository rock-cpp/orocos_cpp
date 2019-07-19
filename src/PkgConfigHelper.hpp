#ifndef PKGCONFIGHELPER_H
#define PKGCONFIGHELPER_H

#include <string>
#include <vector>
#include <map>

namespace orocos_cpp
{

class PkgConfigHelper
{
public:
    static std::vector<std::string> getSearchPathsFromEnvVar();

    /** deprecated
     * Helper function that parses a pkg-Config file. 
     * Will open the file of the given name, and search 
     * for the fields given in searchedFields. The content
     * of the field will be returned in the result vector
     * in the same order as searchedFields. 
     * */
    __attribute__((__deprecated__))
    static bool parsePkgConfig(const std::string &pkgConfigFileName, const std::vector<std::string> &searchedFields, std::vector<std::string> &result, bool is_property=false);
    /**
     * @brief parsePkgConfig
     * @param pkgConfigFileName : filename of pkgconfig file to parse. The file will be search for in all folders specified in PKG_CONFIG_PATH environment variable
     * @param variables : All variables specified in \p pkgConfigFileName as <varname, value>-tuples will be stored here
     * @param properties : All properties specified in \p pkgConfigFileName as <varname, value>-tuples will be stored here
     * @return false if any error occured
     */
    static bool parsePkgConfig(const std::string& filePathOrName, std::map<std::string,std::string>& variables, std::map<std::string, std::string> &properties, bool isFilePath = true);

    /**
    * Helper function, to replace a given string by a given string in an input string.
    * */
    static bool solveString(std::string &input, const std::string &replace, const std::string &by);
};
}//end of namespace
#endif // PKGCONFIGHELPER_H
