#include "register.h"
#include "../login/login_page.h"
#include <gtkmm.h>
#include <iostream>
#include <string>
#include "../bcrypt/bcrypt.h"
#include "../env.hpp"
#include "../db/database.h"

using namespace std;

struct RegisterResult
{
    string message;
    bool success;
};

RegisterResult GetRegister(const string &name, const string &email, const string &passw1, const string &passw2)
{

    if (name.empty() || email.empty() || passw1.empty() || passw2.empty())
    {
        return {" ⛔ Register data isn't valid!", false};
    }

    if (passw1.length() < 5 || passw2.length() < 5)
    {
        return {" ⛔ Passwords must be at least 5 characters long!", false};
    }

    if (passw1 != passw2)
    {
        return {" ⛔ Passwords don't match!", false};
    }

    const char *cstr = passw1.c_str();

    char salt[BCRYPT_HASHSIZE];
    char hash[BCRYPT_HASHSIZE];

    int rounds = stoi(env_vars["BCRYPT_ROUNDS"]);

    // Generating Salt.
    if (bcrypt_gensalt(rounds, salt) != 0)
    {
        return {" ⛔ Failed to generate salt!", false};
    }

    if (bcrypt_hashpw(cstr, salt, hash) != 0)
    {
        return {" ⛔ Failed to hash password!", false};
    }

    if (register_data(name, email, hash))
    {
        return {" ✅ Registration Was Sucessfull", true};
    }
    else
    {
        return {" ⛔ failed :( ", false};
    }
}

Gtk::Widget *create_register_page(Gtk::Window &window)
{

    auto outer = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL);
    outer->set_valign(Gtk::ALIGN_CENTER);
    outer->set_halign(Gtk::ALIGN_CENTER);

    auto frame = Gtk::make_managed<Gtk::Frame>();
    frame->set_shadow_type(Gtk::SHADOW_NONE);
    frame->set_name("register-card");

    auto content = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL);
    content->set_spacing(12);
    content->set_margin_top(12);
    content->set_margin_bottom(12);
    content->set_margin_start(12);
    content->set_margin_end(12);

    auto title = Gtk::make_managed<Gtk::Label>("Create an Account");
    title->set_halign(Gtk::ALIGN_CENTER);
    title->get_style_context()->add_class("title");

    // Labels
    auto name_label = Gtk::make_managed<Gtk::Label>("Full Name");
    auto email_label = Gtk::make_managed<Gtk::Label>("Email Address");
    auto pass1_label = Gtk::make_managed<Gtk::Label>("Password");
    auto pass2_label = Gtk::make_managed<Gtk::Label>("Confirm Password");

    name_label->set_halign(Gtk::ALIGN_START);
    email_label->set_halign(Gtk::ALIGN_START);
    pass1_label->set_halign(Gtk::ALIGN_START);
    pass2_label->set_halign(Gtk::ALIGN_START);

    // Entries
    auto name_entry = Gtk::make_managed<Gtk::Entry>();
    auto email_entry = Gtk::make_managed<Gtk::Entry>();
    auto pass1_entry = Gtk::make_managed<Gtk::Entry>();
    auto pass2_entry = Gtk::make_managed<Gtk::Entry>();

    name_entry->set_size_request(300, 30);
    email_entry->set_size_request(300, 30);
    pass1_entry->set_size_request(300, 30);
    pass2_entry->set_size_request(300, 30);

    pass1_entry->set_visibility(false);
    pass1_entry->set_invisible_char('*');
    pass2_entry->set_visibility(false);
    pass2_entry->set_invisible_char('*');

    // Register button
    auto register_button = Gtk::make_managed<Gtk::Button>("Register");
    register_button->set_name("register-button");
    register_button->set_size_request(300, 40);
    register_button->signal_clicked().connect([&window, name_entry, email_entry, pass1_entry, pass2_entry]()
                                              {
                                                  cout << "Name: " << name_entry->get_text() << endl;
                                                  cout << "Email: " << email_entry->get_text() << endl;
                                                  cout << "Password: " << pass1_entry->get_text() << endl;
                                                  cout << "Confirm: " << pass2_entry->get_text() << endl;

                                                  string name = name_entry->get_text();
                                                  string email = email_entry->get_text();
                                                  string passw1 = pass1_entry->get_text();
                                                  string passw2 = pass2_entry->get_text();
                                                
                                                  RegisterResult result = GetRegister(name, email, passw1, passw2);
                                                  cout << result.message << endl;

                                                  if (result.success)
                                                  {
                                                    show_toast_success(window, result.message);

                                                        // Delay navigation to login page (give time to show the toast)
                                                        Glib::signal_timeout().connect_once([&window]() {
                                                        window.remove(); // remove current widget
                                                        Gtk::Widget *login_ui = create_login_page(window);
                                                        window.add(*login_ui);
                                                        window.set_title("TomFi | Login UI");
                                                        login_ui->show_all();
                                                        }, 1500); // 1.5 seconds    
                                                  }
                                                  else
                                                  {
                                                    show_toast_fail(window, result.message);
                                                  } });

    // Bottom link
    auto bottom_text = Gtk::make_managed<Gtk::Label>("Already have an account?");
    auto sign_in_link = Gtk::make_managed<Gtk::Button>("Sign in");
    sign_in_link->get_style_context()->add_class("link-button");
    sign_in_link->set_relief(Gtk::RELIEF_NONE); // No Border.

    sign_in_link->signal_clicked().connect([&window]()
                                           {
    window.remove(); // remove current widget

    Gtk::Widget *login_ui = create_login_page(window); // new content
    window.add(*login_ui);
    window.set_title("TomFi | Login UI");
    login_ui->show_all(); });

    auto bottom_box = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL);
    bottom_box->set_halign(Gtk::ALIGN_CENTER);
    bottom_box->set_spacing(4);
    bottom_box->pack_start(*bottom_text, Gtk::PACK_SHRINK);
    bottom_box->pack_start(*sign_in_link, Gtk::PACK_SHRINK);

    // Pack everything
    content->pack_start(*title, Gtk::PACK_SHRINK);
    content->pack_start(*name_label, Gtk::PACK_SHRINK);
    content->pack_start(*name_entry, Gtk::PACK_SHRINK);
    content->pack_start(*email_label, Gtk::PACK_SHRINK);
    content->pack_start(*email_entry, Gtk::PACK_SHRINK);
    content->pack_start(*pass1_label, Gtk::PACK_SHRINK);
    content->pack_start(*pass1_entry, Gtk::PACK_SHRINK);
    content->pack_start(*pass2_label, Gtk::PACK_SHRINK);
    content->pack_start(*pass2_entry, Gtk::PACK_SHRINK);
    content->pack_start(*register_button, Gtk::PACK_SHRINK);
    content->pack_start(*bottom_box, Gtk::PACK_SHRINK);

    frame->add(*content);
    outer->pack_start(*frame, Gtk::PACK_SHRINK);

    return outer;
}
