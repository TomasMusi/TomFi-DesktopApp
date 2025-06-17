#include "env.hpp"
#include <fstream>
#include <stdexcept>
#include <iostream>
#include <algorithm> // for std::isspace
using namespace std;

// Trim helper
inline string trim(const string &s)
{
    size_t start = s.find_first_not_of(" \t\r\n");
    size_t end = s.find_last_not_of(" \t\r\n");

    return (start == string::npos) ? "" : s.substr(start, end - start + 1);
}

// Define the global variable here
unordered_map<string, string> env_vars;

void load_env(const string &path)
{
    ifstream file(path);
    if (!file.is_open())
    {
        throw runtime_error("Could not open .env file: " + path);
    }

    string line;
    while (getline(file, line))
    {
        if (line.empty() || line[0] == '#')
            continue;

        size_t pos = line.find('=');
        if (pos == string::npos)
            continue;

        string key = trim(line.substr(0, pos));
        string value = trim(line.substr(pos + 1));
        env_vars[key] = value;
    }

    // Optional debug print
    for (const auto &[k, v] : env_vars)
    {
        cout << "[env] " << k << " = " << v << endl;
    }
}
