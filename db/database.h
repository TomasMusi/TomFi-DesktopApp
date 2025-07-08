#ifndef DATABASE_H
#define DATABASE_H

#include <string>

using namespace std;

bool verify_login(const string &email, const string &password);
bool register_data(const string &name, const string &email, const string &password);
bool card_status_toggle(int user_id);
bool add_funds_balance(int user_id, int amount);

#endif
