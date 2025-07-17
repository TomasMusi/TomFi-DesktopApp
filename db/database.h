#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <gtkmm/window.h>

using namespace std;

struct LoginCheckResult
{
    bool success;
    int user_id;
    bool is_2fa_enabled;
};

LoginCheckResult verify_login(const string &email, const string &password);
bool register_data(const string &name, const string &email, const string &password);
bool card_status_toggle(int user_id);
string decrypt_pin(const string &encrypted_pin_base64, const string &private_key_path);
bool add_funds_balance(int user_id, int amount);
string get_decrypted_pin(string user_input_password, int user_id);
bool store_2fa_secret_to_db(int user_id, const string &secret);
bool create_payment(const int sender_account_id,
                    const string &receiver_card_number,
                    const string &description,
                    const string &receiver_name,
                    const string &category,
                    const string &amount,
                    const string &user_pin);
void show_toast_success(Gtk::Window &parent, const Glib::ustring &message);
void show_toast_fail(Gtk::Window &parent, const Glib::ustring &message);

#endif
