#ifndef DATABASE_H
#define DATABASE_H

#include <string>

using namespace std;

struct LoginCheckResult
{
    bool success;
    int user_id;
    bool is_2fa_enabled;
};

LoginCheckResult verify_login(const std::string &email, const std::string &password);
bool register_data(const string &name, const string &email, const string &password);
bool card_status_toggle(int user_id);
bool add_funds_balance(int user_id, int amount);
string get_decrypted_pin(string user_input_password, int user_id);
bool store_2fa_secret_to_db(int user_id, const string &secret);

#endif
