#include <gtkmm.h>
#include <iostream>
#include "dashboard.h"
#include "Session/Session.hpp"
#include "welcome.h"

using namespace std;

Gtk::Widget *create_dashboard(Gtk::Window &window)
{

    // Check if user is authenticated
    if (!current_session.is_authenticated)
    {
        show_toast_fail(window, "Access Denied: Please log in.");

        // Redirect to welcome page
        Gtk::Widget *welcome_ui = create_welcome(window);
        window.remove(); // remove current content
        window.add(*welcome_ui);
        window.set_title("TomFi | Welcome");
        welcome_ui->show_all();

        // Return something (but it won't be used since window is reset)
        return welcome_ui;
    }

    // Outer vertical wrapper to center vertically
    auto outer_wrapper = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL);
    outer_wrapper->set_valign(Gtk::ALIGN_CENTER);
    outer_wrapper->set_halign(Gtk::ALIGN_CENTER); // Optional, if you want to center horizontally too
    outer_wrapper->set_margin_top(30);
    outer_wrapper->set_margin_bottom(30);

    // White card wrapper
    auto white_frame = Gtk::make_managed<Gtk::Frame>();
    white_frame->set_name("dashboard-card-wrapper");
    white_frame->set_shadow_type(Gtk::SHADOW_NONE);
    white_frame->set_margin_top(10);
    white_frame->set_margin_bottom(10);
    white_frame->set_margin_start(10);
    white_frame->set_margin_end(10);

    // Main layout inside the white frame
    auto main_box = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL);
    main_box->set_valign(Gtk::ALIGN_CENTER);
    main_box->set_size_request(950, -1);
    main_box->set_margin_top(20);
    main_box->set_margin_bottom(20);
    main_box->set_margin_start(20);
    main_box->set_margin_end(20);

    // Sidebar
    auto sidebar = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL);
    sidebar->set_name("sidebar");
    sidebar->set_spacing(20);
    sidebar->set_margin_top(30);
    sidebar->set_margin_bottom(30);
    sidebar->set_margin_start(20);
    sidebar->set_margin_end(20);

    auto app_title = Gtk::make_managed<Gtk::Label>("TomFi");
    app_title->set_name("app-title");
    sidebar->pack_start(*app_title, Gtk::PACK_SHRINK);

    vector<string> nav_items = {"Dashboard", "Transactions", "Mail", "Wallet", "Settings"};
    for (const auto &item : nav_items)
    {
        auto btn = Gtk::make_managed<Gtk::Button>(item);
        btn->set_name("sidebar-button");
        sidebar->pack_start(*btn, Gtk::PACK_SHRINK);
    }

    // Main content
    auto content_box = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL);
    content_box->set_name("dashboard-content");
    content_box->set_spacing(20);
    content_box->set_margin_top(20);
    content_box->set_margin_start(20);
    content_box->set_margin_end(20);

    // Welcome
    auto header = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL);
    auto welcome = Gtk::make_managed<Gtk::Label>("Welcome tomas!");
    welcome->set_name("dashboard-welcome");
    header->pack_start(*welcome, Gtk::PACK_SHRINK);
    content_box->pack_start(*header, Gtk::PACK_SHRINK);

    // Top row
    auto top_row = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL);
    top_row->set_spacing(20);

    auto expenses_card = Gtk::make_managed<Gtk::Frame>("Expenses by category");
    expenses_card->set_name("dashboard-card");
    expenses_card->set_size_request(200, 150);
    expenses_card->set_shadow_type(Gtk::SHADOW_NONE); // removing the border.
    top_row->pack_start(*expenses_card, Gtk::PACK_EXPAND_WIDGET);

    auto transactions_card = Gtk::make_managed<Gtk::Frame>("Transactions");
    transactions_card->set_name("dashboard-card");
    transactions_card->set_size_request(200, 150);
    transactions_card->set_shadow_type(Gtk::SHADOW_NONE);
    top_row->pack_start(*transactions_card, Gtk::PACK_EXPAND_WIDGET);

    auto card_frame = Gtk::make_managed<Gtk::Frame>();
    card_frame->set_name("credit-card-frame");
    card_frame->set_shadow_type(Gtk::SHADOW_NONE);

    auto card_box = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL);
    card_box->set_spacing(5);

    auto balance = Gtk::make_managed<Gtk::Label>("$2073");
    balance->set_name("card-balance");
    card_box->pack_start(*balance, Gtk::PACK_SHRINK);

    auto card_number = Gtk::make_managed<Gtk::Label>("1548 4286 1741 5568");
    card_box->pack_start(*card_number, Gtk::PACK_SHRINK);

    auto expiry = Gtk::make_managed<Gtk::Label>("EXP: 12/24   CVV: ***");
    card_box->pack_start(*expiry, Gtk::PACK_SHRINK);

    auto holder = Gtk::make_managed<Gtk::Label>("Cardholder: tomas");
    card_box->pack_start(*holder, Gtk::PACK_SHRINK);

    auto add_btn = Gtk::make_managed<Gtk::Button>("+ New transaction");
    add_btn->set_name("add-transaction");
    card_box->pack_start(*add_btn, Gtk::PACK_SHRINK);

    card_frame->add(*card_box);
    top_row->pack_start(*card_frame, Gtk::PACK_EXPAND_WIDGET);

    content_box->pack_start(*top_row, Gtk::PACK_SHRINK);

    auto analytics = Gtk::make_managed<Gtk::Frame>("Analytics");
    analytics->set_name("dashboard-card");
    analytics->set_shadow_type(Gtk::SHADOW_NONE);

    analytics->set_size_request(600, 250);
    content_box->pack_start(*analytics, Gtk::PACK_SHRINK);

    // Assemble everything
    main_box->pack_start(*sidebar, Gtk::PACK_SHRINK);
    main_box->pack_start(*content_box);

    white_frame->add(*main_box);
    outer_wrapper->pack_start(*white_frame, Gtk::PACK_SHRINK);

    return outer_wrapper;
}
