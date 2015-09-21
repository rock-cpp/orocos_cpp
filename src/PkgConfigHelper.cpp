#include "PkgConfigHelper.hpp"
#include <string>
#include <boost/filesystem.hpp>
#include <boost/tokenizer.hpp>
#include <fstream>

using namespace orocos_cpp;

bool PkgConfigHelper::solveString(std::string &input, const std::string &replace, const std::string &by)
{
    size_t start_pos = input.find(replace);
    if(start_pos == std::string::npos)
        return false;
    
    input.replace(start_pos, replace.length(), by);
    return true;
}

bool PkgConfigHelper::parsePkgConfig(const std::string& pkgConfigFileName, const std::vector< std::string > &searchedFields, std::vector< std::string > &result)
{
    const char *pkgConfigPath = getenv("PKG_CONFIG_PATH");
    if(!pkgConfigPath)
    {
        throw std::runtime_error("Internal Error, no pkgConfig path found.");
    }
    
    std::string pkgConfigPathS = pkgConfigPath;
    
    boost::char_separator<char> sep(":");
    boost::tokenizer<boost::char_separator<char> > paths(pkgConfigPathS, sep);
    
    std::string searchedPath;
    
    for(const std::string &path: paths)
    {
        std::string candidate = path + "/" + pkgConfigFileName;
        if(boost::filesystem::exists(candidate))
        {
            searchedPath = candidate;
            break;
        }
    }

    if(searchedPath.empty())
    {
        throw std::runtime_error("Error, could not find pkg-config file " + pkgConfigFileName + " in the PKG_CONFIG_PATH");
    }
    
    std::vector<bool> found;
    result.resize(searchedFields.size());
    found.resize(searchedFields.size(), false);
    
    std::ifstream fileStream(searchedPath);
    
    while(!fileStream.eof())
    {
        std::string curLine;
        std::getline(fileStream, curLine);
        for(size_t i = 0; i < searchedFields.size(); i++)
        {
            const std::string searched(searchedFields[i] + "=");
            if(curLine.substr(0, searched.size()) == searched)
            {
                result[i] = curLine.substr(searched.size(), curLine.size());
                found[i] = true;
                
                //check if we found all search values
                bool doReturn = true;
                for(bool f: found)
                {
                    doReturn &= f;
                }
                if(doReturn)
                    return true;
            }
        }        
    }
    
    return false;
}



