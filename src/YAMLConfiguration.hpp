#pragma once

#include "Configuration.hpp"
#include <yaml-cpp/yaml.h>

namespace orocos_cpp
{
void displayMap(const YAML::Node &map, int level = 0);

void printNode(const YAML::Node &node, int level = 0);

ConfigValue *getConfigValue(const YAML::Node &node);
ComplexConfigValue *getMap(const YAML::Node &map);

bool insetMapIntoArray(const YAML::Node &map, ArrayConfigValue &array);
bool insetMapIntoArray(const YAML::Node &map, ComplexConfigValue &complex);
bool insetMapIntoArray(const YAML::Node &map, Configuration &conf);

}