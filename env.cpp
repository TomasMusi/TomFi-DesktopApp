#include "env.hpp"
#include <fstream>
#include <stdexcept>
#include <iostream>

// Define the global variable here
std::unordered_map<std::string, std::string> env_vars;

void load_env(const std::string &path)
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        throw std::runtime_error("Could not open .env file: " + path);
    }

    std::string line;
    while (std::getline(file, line))
    {
        if (line.empty() || line[0] == '#')
            continue;

        size_t pos = line.find('=');
        if (pos == std::string::npos)
            continue;

        std::string key = line.substr(0, pos);
        std::string value = line.substr(pos + 1);
        env_vars[key] = value;
    }

    // Optional debug print
    for (const auto &[k, v] : env_vars)
    {
        std::cout << "[env] " << k << " = " << v << std::endl;
    }
}
