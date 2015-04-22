#ifndef PKGCONFIGHELPER_H
#define PKGCONFIGHELPER_H

#include <string>
#include <vector>

class PkgConfigHelper
{
public:
    /**
     * Helper function that parses a pkg-Config file. 
     * Will open the file of the given name, and search 
     * for the fields given in searchedFields. The content
     * of the field will be returned in the result vector
     * in the same order as searchedFields. 
     * */
    static bool parsePkgConfig(const std::string &pkgConfigFileName, const std::vector<std::string> &searchedFields, std::vector<std::string> &result);

    /**
    * Helper function, to replace a given string by a given string in an input string.
    * */
    static bool solveString(std::string &input, const std::string &replace, const std::string &by);
};

#endif // PKGCONFIGHELPER_H
