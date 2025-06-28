// session.hpp
#ifndef SESSION_HPP
#define SESSION_HPP

#include <string>
#include <vector>

using namespace std;

struct Transaction
{
    string date;        // from timestamp
    string description; // varchar(225)
    string category;    // varchar(50)
    string amount;      // varchar(50)
    string direction;   // enum('in','out')
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

    // Transactions
    vector<Transaction> transactions;
};

extern Session current_session;

#endif
