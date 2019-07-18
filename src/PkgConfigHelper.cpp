#include "PkgConfigHelper.hpp"
#include <string>
#include <boost/filesystem.hpp>
#include <boost/tokenizer.hpp>
#include <fstream>
#include <regex>
#include <boost/algorithm/string.hpp>
#include <iostream>

using namespace orocos_cpp;


bool PkgConfigHelper::solveString(std::string &input, const std::string &replace, const std::string &by)
{
    size_t start_pos = input.find(replace);
    if(start_pos == std::string::npos)
        return false;
    
    input.replace(start_pos, replace.length(), by);
    return true;
}



//!
//! \brief substitudes all occurences of a varibale in \p line by it's substitudes given in \p variables
//! \param line : The input line that can contain variables. A variable is identified by beeing enclosed by 2${}". e.g. ${varname})
//! \param variables : A map containing <varname, value used for substitution>-tuples
//! \param substituded : Substituded version of \p line will be stored here
//! \return true if all varibales where successfully substituded. False if a varibale could not be resolved.
//!
bool substitude(const std::string& line, const std::map<std::string, std::string>& variables, std::string& substituded)
{
    std::regex e{R"(.*\$\{(\w*)\}.*)"};
    std::smatch sm;
    std::cout <<"====" <<std::endl;
    std::cout << line <<std::endl;
    std::string new_line = line;
    bool ok = true;
    while(std::regex_match(new_line, sm, e)){
        std::string varname = sm[1];
        std::string varstr = "${"+varname+"}";
        try{
            std::string val = variables.at(varname);
            size_t pos = new_line.find(varstr);
            new_line.replace(pos, varstr.size(), val);
        }catch(std::out_of_range ex){
            std::clog << "Could not substitude varibale " << varname << std::endl;
            ok=false;
        }
    }
    substituded = new_line;
    std::cout << substituded <<std::endl;
    std::cout <<"...." <<std::endl;
    return ok;
}

//!
//! \brief Identifies if the given \p line is a PKGConfig comment
//! \param line
//! \return true if \p line is a comment. false otherwise.
//!
bool parseComment(const std::string& line)
{
    //Is comment if first character is '#'
    if(line.size() < 1)
        return false;
    return line.at(0) == '#';
}

//!
//! \brief Identifies if the given \p line is a variable assignment
//! Varibales in PKGConfig can have arbitrary names. They are assigned by "varname=value".
//! Example:
//!     prefix=/my/prefix
//!     exec_prefix=${prefix}
//!     libdir=${prefix}/lib/orocos/types
//! \param line
//! \return true if \p line is a varibale assignment. false otherwise.
//!
bool parseVariable(const std::string& line, std::string& var_name, std::string& value)
{
    //Start of line, followed by a word, followed by a '=' sign, NO whitespace
    std::regex e("^(\\w+)=(.*)");
    std::smatch sm;
    if(std::regex_match(line, sm, e)){
        assert(sm.size() == 3);
        var_name = sm[1];
        value = sm[2];
        return true;
    }
    return false;
}

//!
//! \brief Identifies if the given \p line is a property assignment
//! //! In PKGConfig there exists a defined set of properties to describe a software module for compiling and linking. They are assigned by "propname: value".
//! Example:
//!    Name: testTypekit
//!    Version: 0.0
//!    Requires: test
//!    Description: test types support for the Orocos type system
//!    Libs: -L${libdir} -ltest-typekit-gnulinux
//!    Cflags: -I${includedir} -I${includedir}/test/types "-DOROCOS_TARGET=gnulinux"
//! \param line
//! \return true if \p line is a comment. false otherwise.
//!
bool parseProperty(const std::string& line, std::string& prop_name, std::string& value)
{
    //Start of line, followed by a word, followed by a '=' sign, followed by a whitespace
    std::regex e("^(\\w+):\\s(.*)");
    std::smatch sm;
    if(std::regex_match(line, sm, e)){
        assert(sm.size() == 3);
        prop_name = sm[1];
        value = sm[2];
        return true;
    }
    return false;
}

bool PkgConfigHelper::parsePkgConfig(const std::string& pkgConfigFileName, std::map<std::string,std::string>& variables, std::map<std::string,std::string>& properties)
{
    //Resolve full filepath for filename given via pkgConfigFileName
    std::vector<std::string> paths = get_search_paths_from_env_var();
    std::string filepath = find_file(pkgConfigFileName, paths);

    if(filepath.empty())
    {
        throw std::runtime_error("Error, could not find pkg-config file " + pkgConfigFileName + " in the PKG_CONFIG_PATH");
    }

    std::ifstream fileStream(filepath);
    bool all_ok=true;
    while(!fileStream.eof())
    {
        std::string curLine;
        std::getline(fileStream, curLine);
        if(parseComment(curLine))
            continue;

        std::string name, value;
        if(parseVariable(curLine, name, value)){
            variables[name] = value;
            continue;
        }
        if(parseProperty(curLine, name, value)){
            properties[name] = value;
            continue;
        }

        //Not a comment, property or variable.. check if empty (only whitespaces and print warning otherwise)
        std::string curLine2 = curLine;
        std::string::iterator end_pos = std::remove(curLine2.begin(), curLine2.end(), ' '); //Removes all whitespaces
        curLine2.erase(end_pos, curLine2.end());

        //Deletes all whitespaces
        if(!curLine2.empty()){
            std::clog << "Warning: Could not parse line " << curLine << " from PkgConfig-file " << filepath << std::endl;
            all_ok = false;
        }
    }

    //Substitude field values with values from variables
    for(std::pair<const std::string, std::string>& var : variables){
        bool st = substitude(var.second, variables, var.second);
        if(!st){
            std::clog << "Could not substitude value '" << var.second << "' of variable " << var.first << " from PkgConfig-file " << filepath << std::endl;
            all_ok = false;
        }
    }
    for(std::pair<const std::string, std::string>& prop : properties){
        bool st = substitude(prop.second, variables, prop.second);
        if(!st){
            std::clog << "Could not substitude value " << prop.second << "' of property "<< prop.first << " from PkgConfig-file " << filepath << std::endl;
            all_ok = false;
        }
    }
    return all_ok;
}

bool PkgConfigHelper::parsePkgConfig(const std::string& pkgConfigFileName, const std::vector< std::string > &searchedFields, std::vector< std::string > &result, bool is_property)
{
    std::map<std::string,std::string> variables;
    std::map<std::string,std::string> properties;
    parsePkgConfig(pkgConfigFileName, variables, properties);

    bool all_ok = true;
    result.resize(searchedFields.size());
    for(size_t i=0; i<searchedFields.size(); i++){
        const std::string& fname = searchedFields[i];
        std::map<std::string,std::string>::iterator it;
        std::map<std::string,std::string>::iterator not_found;

        if(is_property){
            it = properties.find(fname);
            not_found = properties.end();
        }
        else{
            it = variables.find(fname);
            not_found = variables.end();
        }
        if(it == not_found)
            all_ok &= false;
        else{
            all_ok &= true;
            result[i] = it->second;
        }
    }
    return all_ok;
}



