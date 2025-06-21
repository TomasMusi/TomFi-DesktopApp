// session.hpp
#ifndef SESSION_HPP
#define SESSION_HPP

#include <string>

using namespace std;

struct Session
{
    bool is_authenticated = false;
    int user_id = -1;
    string name;
    string email;
    string role;
};

extern Session current_session;

#endif
