// session.hpp
#ifndef SESSION_HPP
#define SESSION_HPP

#include <string>
#include <vector>

using namespace std;

struct Transaction
{
    string description;
    string amount;
    string direction;
};

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

    // Transaction

    vector<Transaction> transactions;
};

extern Session current_session;

#endif
