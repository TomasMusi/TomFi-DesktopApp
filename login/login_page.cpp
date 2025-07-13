#include "login_page.h"
#include <iostream>
#include "../register/register.h"
#include "../db/database.h"
#include "../dashboard/dashboard.h"
#include "2fa-page/2fa_verify.h"

using namespace std;

struct LoginResults
{
    string message;
    bool success;
    int user_id;
    bool is_2fa_enabled;
};
LoginResults LoginRequest(string &email, string &password)
{
    if (email.empty() || password.empty())
    {
        return {" ⛔ Login data are not valid!", false, -1, false};
    }

    LoginCheckResult result = verify_login(email, password);
    if (result.success)
    {
        return {" ✅ Login Successful", true, result.user_id, result.is_2fa_enabled};
    }
    else
    {
        return {" ⛔ Login failed", false, -1, false};
    }
}

Gtk::Widget *
create_login_page(Gtk::Window &window)
{

    auto outer = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL);
    outer->set_valign(Gtk::ALIGN_CENTER);
    outer->set_halign(Gtk::ALIGN_CENTER);

    auto frame = Gtk::make_managed<Gtk::Frame>();
    frame->set_shadow_type(Gtk::SHADOW_NONE);
    frame->set_name("login-card");

    auto content = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL);
    content->set_spacing(12);
    content->set_margin_top(12);
    content->set_margin_bottom(12);
    content->set_margin_start(12);
    content->set_margin_end(12);

    auto title = Gtk::make_managed<Gtk::Label>("Welcome Back");
    title->set_halign(Gtk::ALIGN_CENTER);
    title->get_style_context()->add_class("title");

    auto email_label = Gtk::make_managed<Gtk::Label>("Email Address");
    auto email_entry = Gtk::make_managed<Gtk::Entry>();
    email_entry->set_placeholder_text("example@domain.com");
    email_entry->set_size_request(300, 30);

    auto pass_label = Gtk::make_managed<Gtk::Label>("Password");
    auto pass_entry = Gtk::make_managed<Gtk::Entry>();
    pass_entry->set_placeholder_text("**");
    pass_entry->set_visibility(false);
    pass_entry->set_invisible_char('*');
    pass_entry->set_size_request(300, 30);

    auto forgot = Gtk::make_managed<Gtk::Label>("Forgot your password?");
    forgot->get_style_context()->add_class("link");
    forgot->set_halign(Gtk::ALIGN_END);

    email_label->set_halign(Gtk::ALIGN_START);
    pass_label->set_halign(Gtk::ALIGN_START);

    auto login_button = Gtk::make_managed<Gtk::Button>("Log In");
    login_button->set_name("login-button");
    login_button->set_size_request(300, 40);
    login_button->signal_clicked().connect([&window, email_entry, pass_entry]()
                                           {
    string Email = email_entry->get_text();
    string Password = pass_entry->get_text();

    LoginResults result = LoginRequest(Email, Password);

    if (result.success) {
        show_toast_success(window, result.message);

        window.remove();

        if (result.is_2fa_enabled) {
            // 🔐 Redirect to 2FA screen
            Gtk::Widget *twofa_ui = create_2fa_page(window, result.user_id);
            window.add(*twofa_ui);
            window.set_title("TomFi | Enter 2FA Code");
            twofa_ui->show_all();
        } else {
            // 🚀 Go to Dashboard directly
            Gtk::Widget *dashboard_ui = create_dashboard(window);
            window.add(*dashboard_ui);
            window.set_title("TomFi | Dashboard");
            dashboard_ui->show_all();
        }
    } else {
        show_toast_fail(window, result.message);
    } });

    auto register_text = Gtk::make_managed<Gtk::Label>("Don't have an account?");
    auto register_link = Gtk::make_managed<Gtk::Button>("Register");
    register_link->get_style_context()->add_class("link-button");
    register_link->set_relief(Gtk::RELIEF_NONE); // No border

    register_link->signal_clicked().connect([&window]()
                                            {
    window.remove(); // remove current widget

    Gtk::Widget *register_ui = create_register_page(window); // new content
    window.add(*register_ui);
    window.set_title("TomFi | Register UI");
    register_ui->show_all(); });

    auto register_box = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL);
    register_box->set_halign(Gtk::ALIGN_CENTER);
    register_box->set_spacing(4);
    register_box->pack_start(*register_text, Gtk::PACK_SHRINK);
    register_box->pack_start(*register_link, Gtk::PACK_SHRINK);

    content->pack_start(*title, Gtk::PACK_SHRINK);
    content->pack_start(*email_label, Gtk::PACK_SHRINK);
    content->pack_start(*email_entry, Gtk::PACK_SHRINK);
    content->pack_start(*pass_label, Gtk::PACK_SHRINK);
    content->pack_start(*pass_entry, Gtk::PACK_SHRINK);
    content->pack_start(*forgot, Gtk::PACK_SHRINK);
    content->pack_start(*login_button, Gtk::PACK_SHRINK);
    content->pack_start(*register_box, Gtk::PACK_SHRINK);

    frame->add(*content);
    outer->pack_start(*frame, Gtk::PACK_SHRINK);

    return outer;
}
