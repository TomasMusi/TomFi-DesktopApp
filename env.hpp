#pragma once

#include <string>
#include <unordered_map>

// Declare a global variable (extern means it's defined elsewhere)
extern std::unordered_map<std::string, std::string> env_vars;

// Loads from file into env_vars
void load_env(const std::string &path);
