// session.hpp
#ifndef SESSION_HPP
#define SESSION_HPP

#include <string>

using namespace std;

struct Session
{
    // User
    bool is_authenticated = false;
    int user_id = -1;
    string name;
    string email;
    string role;

    // Card
    int is_active = 1;
    string balance;
    string card_number;
};

extern Session current_session;

#endif
