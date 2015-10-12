#include "YAMLConfiguration.hpp"
#include <boost/filesystem.hpp>
#include <boost/algorithm/string/regex.hpp>
#include "Bundle.hpp"
#include <fstream>

using namespace orocos_cpp;

void YAMLConfigParser::printNode(const YAML::Node &node, int level)
{
    for(int i = 0; i < level; i++)
        std::cout << "  ";

    switch(node.Type())
    {
        case YAML::NodeType::Scalar:
        {
            std::string value;
            if(node.GetScalar(value))
            {
                std::cout << "a Scalar: "<< value << std::endl;
            }
            else
                std::cout << "a Scalar: without value " << std::endl;
        }
            break;
        case YAML::NodeType::Sequence:
            std::cout << "a Sequence: " << node.Tag() << std::endl;
            break;
        case YAML::NodeType::Map:
            std::cout << "a Map: " << node.Tag() << std::endl;
            displayMap(node, level + 1);
            break;
        case YAML::NodeType::Null:
            break;
    }
}

bool YAMLConfigParser::insetMapIntoArray(const YAML::Node& map, ComplexConfigValue& array)
{
    for(YAML::Iterator it = map.begin(); it != map.end(); it++)
    {
        std::string memberName;
        it.first() >> memberName;
//         std::cout << "Name : " << memberName << std::endl;
        std::shared_ptr<ConfigValue> val = getConfigValue(it.second());
        
        if(!val)
        {
            std::cout << "Warning, could not get config value for " << memberName << std::endl;
            continue;
        }
        
        val->setName(memberName);

        array.addValue(memberName, val);
    }
    return !array.getValues().empty();
}


bool YAMLConfigParser::insetMapIntoArray(const YAML::Node &map, ArrayConfigValue &array)
{
    for(YAML::Iterator it = map.begin(); it != map.end(); it++)
    {
        std::string memberName;
        it.first() >> memberName;
//         std::cout << "Name : " << memberName << std::endl;
        std::shared_ptr<ConfigValue> val = getConfigValue(it.second());
        
        if(!val)
        {
            std::cout << "Warning, could not get config value for " << memberName << std::endl;
            continue;
        }
        
        val->setName(memberName);

        array.addValue(val);
    }
    return !array.getValues().empty();
}

bool YAMLConfigParser::insetMapIntoArray(const YAML::Node& map, Configuration& conf)
{
    for(YAML::Iterator it = map.begin(); it != map.end(); it++)
    {
        std::string memberName;
        it.first() >> memberName;
//         std::cout << "Name : " << memberName << std::endl;
        std::shared_ptr<ConfigValue> val = getConfigValue(it.second());
        
        if(!val)
        {
            std::cout << "Warning, could not get config value for " << memberName << std::endl;
            continue;
        }
        
        val->setName(memberName);

        conf.addValue(memberName, val);
    }
    return !conf.getValues().empty();

}       

std::shared_ptr<ConfigValue> YAMLConfigParser::getConfigValue(const YAML::Node &node)
{
    switch(node.Type())
    {
        case YAML::NodeType::Scalar:
        {
            std::string value;
            if(node.GetScalar(value))
            {
//                 std::cout << "a Scalar: "<< value << std::endl;
                std::shared_ptr<ConfigValue> conf;
                conf.reset(new SimpleConfigValue(value));
                return conf;
            }
            else
                std::cout << "a Scalar: without value " << std::endl;
        }
            break;
        case YAML::NodeType::Sequence:
//             std::cout << "a Sequence: " << node.Tag() << std::endl;
            {
                std::shared_ptr<ArrayConfigValue> values = std::make_shared<ArrayConfigValue>();
                for(YAML::Iterator it = node.begin(); it != node.end(); it++)
                {
                    std::shared_ptr<ConfigValue> curConf = getConfigValue(*it);
                    
                    values->addValue(curConf);
                }
                return values;
            }
            break;
        case YAML::NodeType::Map:
        {
//             std::cout << "a Map: " << node.Tag() << std::endl;
            std::shared_ptr<ComplexConfigValue> mapValue = std::make_shared<ComplexConfigValue>();
            if(!insetMapIntoArray(node, *mapValue))
            {
                return std::shared_ptr<ConfigValue>();
            }
            return mapValue;
            break;
        }
        case YAML::NodeType::Null:
            std::cout << "NULL" << std::endl;
            break;
    }
    return nullptr;
}

std::shared_ptr<ComplexConfigValue> YAMLConfigParser::getMap(const YAML::Node &map)
{
    std::shared_ptr<ComplexConfigValue> mapValue = std::make_shared<ComplexConfigValue>();
    if(!insetMapIntoArray(map, *mapValue))
    {
        return nullptr;
    }

    return mapValue;
}

void YAMLConfigParser::displayMap(const YAML::Node &map, int level)
{
    for(YAML::Iterator it = map.begin(); it != map.end(); it++)
    {
        for(int i = 0; i < level; i++)
            std::cout << "  ";
        
        std::string value;
        it.first() >> value;
        std::cout << "Value of first " << value << std::endl;
        printNode(it.second(), level + 1);
    }
    
}

bool YAMLConfigParser::loadConfigFile(const std::string& pathStr, std::map<std::string, Configuration> &subConfigs)
{
    subConfigs.clear();
    using namespace boost::filesystem;
    
    path path(pathStr);

    if(!exists(path))
    {
        throw std::runtime_error(std::string("Error, could not find config file ") + path.c_str());
    }

    //as this is non standard yml, we need to load and parse the config file first
    std::ifstream fin(path.c_str());
    std::string line;
    
    std::string buffer;
    
    Configuration curConfig("dummy");
    bool hasConfig = false;

    
    while(std::getline(fin, line))
    {
        if(line.size() >= 3 && line.at(0) == '-'  && line.at(1) == '-'  && line.at(2) == '-' )
        {
            //found new subsection
//             std::cout << "found subsection " << line << std::endl;
            
            std::string searched("--- name:");
            if(!line.compare(0, searched.size(), searched))
            {

                if(hasConfig)
                {
                    if(!parseYAML(curConfig, applyStringVariableInsertions(buffer)))
                        return false;
                    buffer.clear();
                    subConfigs.insert(std::make_pair(curConfig.getName(), curConfig));
                }
                
                hasConfig = true;
                
                curConfig = Configuration(line.substr(searched.size(), line.size()));

//                 std::cout << "Found new configuration " << curConfig.name << std::endl;
            } else
            {
                std::cout << "Error, sections must begin with '--- name:<SectionName>'" << std::endl;
                return false;
            }

        } else
        {
            //line belongs to the last detected section, add it to the buffer
            buffer.append(line);
            buffer.append("\n");
        }
    }
    
    if(hasConfig)
    {
        if(!parseYAML(curConfig, applyStringVariableInsertions(buffer)))
            return false;
        subConfigs.insert(std::make_pair(curConfig.getName(), curConfig));
    }

//     for(std::map<std::string, Configuration>::const_iterator it = subConfigs.begin(); it != subConfigs.end(); it++)
//     {
//         std::cout << "Cur conf \"" << it->first << "\"" << std::endl;
//         displayConfiguration(it->second);
//     }
    
    return true;
}

bool YAMLConfigParser::parseYAML(Configuration& curConfig, const std::string& yamlBuffer)
{
    std::stringstream fin(yamlBuffer);
    YAML::Parser parser(fin);

    YAML::Node doc;
    
    while(parser.GetNextDocument(doc)) {

        if(doc.Type() != YAML::NodeType::Map)
        {
            std::cout << "Error, configurations section should only contain yml maps" << std::endl;
            return false;
        }
        
        if(doc.Type() == YAML::NodeType::Map)
        {
            if(!insetMapIntoArray(doc, curConfig))
            {
                std::cout << "Warning, could not parse config" << std::endl;
            }
        }
    }
    
    return true;
}

std::string YAMLConfigParser::applyStringVariableInsertions(const std::string& val)
{
    std::string retVal = "";
    std::vector<std::string> items;
    //parsing begin and end of code block
    boost::algorithm::split_regex(items, val, boost::regex("<%[=\\s]*|[\\s%]*>"));

    for(auto &elem: items){
        //searching for keywords we want to support (ENV or BUNDLES)
        if(elem.find("ENV") != std::string::npos){
                //replace environment variables:
                std::vector<std::string> variables;
                boost::algorithm::split_regex(variables, elem, boost::regex("ENV[\\[\\(] ?['\"]?|['\"]? ?[\\]\\)]"));
                for(auto &var: variables){
                        if(var.empty()){
                                continue;
                        }
                        //add variable to output string:
                        retVal += std::getenv(var.c_str());
                }
        }else if(elem.find("BUNDLES") != std::string::npos){
                //if bundles search for path in the bundles. The path is given as parameter for BUNDLES:
                std::vector<std::string> variables;
                boost::algorithm::split_regex(variables, elem, boost::regex("BUNDLES[\\[\\(] ?['\"]?|['\"]? ?[\\]\\)]"));
                for(auto &val: variables){
                        if(val.empty())
                                continue;
                    retVal += Bundle::getInstance().findFile(val);
                }
        }else{
                retVal += elem;
        }
    }

//      std::cout << "Returning String: '" << retVal << "'" << std::endl;

    return retVal;
}
