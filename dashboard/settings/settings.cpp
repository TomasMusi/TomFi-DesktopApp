#include <gtkmm.h>
#include <string>
#include "Session/Session.hpp"
#include "settings.h"
#include "dashboard/wallet/wallet.h"
#include "dashboard/dashboard.h"
#include <iostream>
#include "welcome.h"

using namespace std;

Gtk::Widget *create_settings(Gtk::Window &window)
{
    // Middlware to check, if the user is loggined.
    if (!current_session.is_authenticated)
    {
        show_toast_fail(window, "Access Denied: Please log in.");
        Gtk::Widget *welcome_ui = create_welcome(window);
        window.remove();
        window.add(*welcome_ui);
        window.set_title("TomFi | Welcome");
        welcome_ui->show_all();
        return welcome_ui;
    }

    auto outer_wrapper = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL);
    outer_wrapper->set_valign(Gtk::ALIGN_CENTER);
    outer_wrapper->set_halign(Gtk::ALIGN_CENTER);
    outer_wrapper->set_margin_top(30);
    outer_wrapper->set_margin_bottom(30);

    auto white_frame = Gtk::make_managed<Gtk::Frame>();
    white_frame->set_name("dashboard-card-wrapper");
    white_frame->set_shadow_type(Gtk::SHADOW_NONE);
    white_frame->set_margin_top(10);
    white_frame->set_margin_bottom(10);
    white_frame->set_margin_start(10);
    white_frame->set_margin_end(10);

    auto main_box = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL);
    main_box->set_name("dashboard-wrapper");
    main_box->set_valign(Gtk::ALIGN_CENTER);
    main_box->set_size_request(950, -1);
    main_box->set_margin_top(20);
    main_box->set_margin_bottom(20);
    main_box->set_margin_start(20);
    main_box->set_margin_end(20);

    auto sidebar_wrapper = Gtk::make_managed<Gtk::Frame>();
    sidebar_wrapper->set_name("sidebar-wrapper");
    sidebar_wrapper->set_shadow_type(Gtk::SHADOW_NONE);

    auto sidebar = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL);
    sidebar->set_name("sidebar");
    sidebar->set_spacing(20);
    sidebar->set_margin_top(30);
    sidebar->set_margin_bottom(30);
    sidebar->set_margin_start(20);
    sidebar->set_margin_end(20);
    sidebar->set_valign(Gtk::ALIGN_FILL);

    auto app_title = Gtk::make_managed<Gtk::Label>("TomFi");
    app_title->set_name("app-title");
    sidebar->pack_start(*app_title, Gtk::PACK_SHRINK);

    // Dashboard SideButton
    auto dashboard_btn = Gtk::make_managed<Gtk::Button>("Dashboard");
    dashboard_btn->set_name("sidebar-button");
    dashboard_btn->set_halign(Gtk::ALIGN_START);
    dashboard_btn->signal_clicked().connect([&window]()
                                            {
        Gtk::Widget *dashboard_ui = create_dashboard(window);
        window.remove();
        window.add(*dashboard_ui);
        window.set_title("TomFi | Dashboard");
        dashboard_ui->show_all(); });
    sidebar->pack_start(*dashboard_btn, Gtk::PACK_SHRINK);

    // Transaction SideButton
    auto transactions_btn = Gtk::make_managed<Gtk::Button>("Transactions");
    transactions_btn->set_name("sidebar-button");
    transactions_btn->set_halign(Gtk::ALIGN_START);
    sidebar->pack_start(*transactions_btn, Gtk::PACK_SHRINK);

    // Mail SideButton
    auto mail_btn = Gtk::make_managed<Gtk::Button>("Mail");
    mail_btn->set_name("sidebar-button");
    mail_btn->set_halign(Gtk::ALIGN_START);
    sidebar->pack_start(*mail_btn, Gtk::PACK_SHRINK);

    // Wallet SideButton
    auto wallet_btn = Gtk::make_managed<Gtk::Button>("Wallet");
    wallet_btn->set_name("sidebar-button");
    wallet_btn->set_halign(Gtk::ALIGN_START);
    wallet_btn->signal_clicked().connect([&window]()
                                         {
        Gtk::Widget *wallet_ui = create_wallet_ui(window);
        window.remove();
        window.add(*wallet_ui);
        window.set_title("TomFi | Wallet");
        wallet_ui->show_all(); });
    sidebar->pack_start(*wallet_btn, Gtk::PACK_SHRINK);

    // Settings SideButton
    auto settings_btn = Gtk::make_managed<Gtk::Button>("Settings");
    settings_btn->set_name("side-button-current"); // giving settings button extra class, because i want it to be blue.
    settings_btn->set_halign(Gtk::ALIGN_START);
    sidebar->pack_start(*settings_btn, Gtk::PACK_SHRINK);

    sidebar_wrapper->add(*sidebar);
    main_box->pack_start(*sidebar_wrapper, Gtk::PACK_SHRINK);

    auto content_box = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL);
    content_box->set_name("settings-content-box");
    content_box->set_spacing(20);
    content_box->set_margin_top(20);
    content_box->set_margin_start(20);
    content_box->set_margin_end(20);

    // Content
    auto info_title = Gtk::make_managed<Gtk::Label>("Personal Information");
    info_title->set_name("info-title");
    info_title->set_halign(Gtk::ALIGN_CENTER);
    content_box->pack_start(*info_title, Gtk::PACK_SHRINK);

    auto form_grid = Gtk::make_managed<Gtk::Grid>();
    form_grid->set_row_spacing(10);
    form_grid->set_column_spacing(10);
    form_grid->set_margin_top(10);
    form_grid->set_margin_bottom(10);

    int row = 0;

    auto name_label = Gtk::make_managed<Gtk::Label>("Full Name");
    name_label->set_halign(Gtk::ALIGN_START);
    auto name_entry = Gtk::make_managed<Gtk::Entry>();
    name_entry->set_text(current_session.name);
    name_entry->set_editable(false);
    form_grid->attach(*name_label, 0, row, 1, 1);
    form_grid->attach(*name_entry, 1, row++, 1, 1);

    auto email_label = Gtk::make_managed<Gtk::Label>("Email Address");
    email_label->set_halign(Gtk::ALIGN_START);
    auto email_entry = Gtk::make_managed<Gtk::Entry>();
    email_entry->set_text(current_session.email);
    form_grid->attach(*email_label, 0, row, 1, 1);
    form_grid->attach(*email_entry, 1, row++, 1, 1);

    auto phone_label = Gtk::make_managed<Gtk::Label>("Phone Number");
    phone_label->set_halign(Gtk::ALIGN_START);
    auto phone_entry = Gtk::make_managed<Gtk::Entry>();
    phone_entry->set_text("+1 123 456 7890");
    form_grid->attach(*phone_label, 0, row, 1, 1);
    form_grid->attach(*phone_entry, 1, row++, 1, 1);

    auto addr_label = Gtk::make_managed<Gtk::Label>("Address");
    addr_label->set_halign(Gtk::ALIGN_START);
    auto addr_entry = Gtk::make_managed<Gtk::TextView>();
    addr_entry->get_buffer()->set_text("1234 Elm Street, Springfield, USA");
    addr_entry->set_size_request(300, 60);
    form_grid->attach(*addr_label, 0, row, 1, 1);
    form_grid->attach(*addr_entry, 1, row++, 1, 1);

    auto dob_label = Gtk::make_managed<Gtk::Label>("Date of Birth");
    dob_label->set_halign(Gtk::ALIGN_START);
    auto dob_entry = Gtk::make_managed<Gtk::Entry>();
    dob_entry->set_text("01 / 01 / 1990");
    dob_entry->set_editable(false);
    form_grid->attach(*dob_label, 0, row, 1, 1);
    form_grid->attach(*dob_entry, 1, row++, 1, 1);

    content_box->pack_start(*form_grid, Gtk::PACK_SHRINK);

    auto control_box = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL);
    control_box->set_halign(Gtk::ALIGN_FILL);
    control_box->set_spacing(10);

    auto signout_btn = Gtk::make_managed<Gtk::Button>("Sign Out");
    signout_btn->set_name("signout-btn");

    auto save_btn = Gtk::make_managed<Gtk::Button>("Save Changes");
    save_btn->set_name("save-btn");

    control_box->pack_start(*signout_btn, Gtk::PACK_SHRINK);
    control_box->pack_end(*save_btn, Gtk::PACK_SHRINK);
    content_box->pack_start(*control_box, Gtk::PACK_SHRINK);

    signout_btn->signal_clicked().connect([&window]()
                                          {
        std::cout << "🔓 Signed out." << std::endl;
        Gtk::MessageDialog dialog(window, "You have been signed out.", false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK, true);
        dialog.run(); });

    save_btn->signal_clicked().connect([]()
                                       { cout << "✅ Save button pressed." << endl; });

    auto center_wrapper = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL);
    center_wrapper->set_halign(Gtk::ALIGN_CENTER);
    center_wrapper->set_hexpand(true);

    center_wrapper->pack_start(*content_box, Gtk::PACK_SHRINK);
    main_box->pack_start(*center_wrapper, Gtk::PACK_EXPAND_WIDGET);

    white_frame->add(*main_box);
    outer_wrapper->pack_start(*white_frame);

    return outer_wrapper;
}