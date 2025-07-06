#include <gtkmm.h>
#include <iostream>
#include <string>
#include "dashboard/dashboard.h"
#include "dashboard/chart/chart.h"
#include "Session/Session.hpp"
#include "welcome.h"
#include "dashboard/Transaction/transaction.h"
#include "dashboard/chart/longChart.h"
#include "wallet.h"

using namespace std;

Gtk::Widget *create_wallet_ui(Gtk::Window &window)
{
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

    string card_holder_text = "CardHolder: " + current_session.name;
    string card_number = current_session.card_number;
    string card_balance = "$" + current_session.balance;

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

    vector<string> nav_items = {"Dashboard", "Transactions", "Mail", "Wallet", "Settings"};
    for (const auto &name : nav_items)
    {
        auto btn = Gtk::make_managed<Gtk::Button>(name);
        btn->set_name(name == "Wallet" ? "side-button-current" : "sidebar-button");
        btn->set_halign(Gtk::ALIGN_START);

        btn->signal_clicked().connect([&, name]()
                                      {
            Gtk::Widget *target_ui = nullptr;

            if (name == "Dashboard")
                target_ui = create_dashboard(window);
            else if (name == "Transactions")
                target_ui = create_transactions_ui(window);
            else if (name == "Wallet")
                target_ui = create_wallet_ui(window);
            else if (name == "Mail")
            {
                show_toast_fail(window, "Mail page coming soon.");
                return;
            }
            else if (name == "Settings")
            {
                show_toast_fail(window, "Settings not implemented yet.");
                return;
            }

            if (target_ui)
            {
                window.remove();
                window.add(*target_ui);
                window.set_title("TomFi | " + name);
                target_ui->show_all();
            } });

        sidebar->pack_start(*btn, Gtk::PACK_SHRINK);
    }

    sidebar_wrapper->add(*sidebar);
    main_box->pack_start(*sidebar_wrapper, Gtk::PACK_SHRINK);

    auto content_box = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL);
    content_box->set_name("dashboard-content");
    content_box->set_spacing(20);
    content_box->set_margin_top(20);
    content_box->set_margin_start(20);
    content_box->set_margin_end(20);

    auto card_frame = Gtk::make_managed<Gtk::Frame>();
    card_frame->set_name("credit-card-frame");
    card_frame->set_shadow_type(Gtk::SHADOW_NONE);

    auto card_box = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL);
    card_box->set_spacing(5);

    auto balance_row = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL);
    balance_row->set_spacing(10);

    auto balance = Gtk::make_managed<Gtk::Label>(card_balance);
    balance->set_name("card-balance");
    balance->set_halign(Gtk::ALIGN_START);
    balance->set_hexpand(true);

    auto active_badge = Gtk::make_managed<Gtk::Label>(current_session.is_active ? "✅ Active" : "❌ Deactivated");
    active_badge->set_name("active-badge");
    active_badge->set_margin_top(3);
    active_badge->set_halign(Gtk::ALIGN_END);
    if (current_session.is_active)
        active_badge->get_style_context()->add_class("active-badge");
    else
        active_badge->get_style_context()->add_class("inactive-badge");

    balance_row->pack_start(*balance, Gtk::PACK_EXPAND_WIDGET);
    balance_row->pack_start(*active_badge, Gtk::PACK_SHRINK);
    card_box->pack_start(*balance_row, Gtk::PACK_SHRINK);

    auto card_number_label = Gtk::make_managed<Gtk::Label>(card_number);
    card_box->pack_start(*card_number_label, Gtk::PACK_SHRINK);

    auto expiry = Gtk::make_managed<Gtk::Label>("EXP: 12/24   CVV: ***");
    card_box->pack_start(*expiry, Gtk::PACK_SHRINK);

    auto holder = Gtk::make_managed<Gtk::Label>(card_holder_text);
    card_box->pack_start(*holder, Gtk::PACK_SHRINK);

    auto add_btn = Gtk::make_managed<Gtk::Button>("+ New transaction");
    add_btn->set_name("add-transaction");
    card_box->pack_start(*add_btn, Gtk::PACK_SHRINK);

    card_frame->add(*card_box);
    content_box->pack_start(*card_frame, Gtk::PACK_SHRINK);

    auto actions_frame = Gtk::make_managed<Gtk::Frame>("Wallet Actions");
    actions_frame->set_name("dashboard-card");
    actions_frame->set_shadow_type(Gtk::SHADOW_NONE);
    actions_frame->set_size_request(600, 250);

    auto grid = Gtk::make_managed<Gtk::Grid>();
    grid->set_column_spacing(16);
    grid->set_row_spacing(10);

    vector<pair<string, string>> actions = {
        {"Transfer", "system-run"},
        {"Show PIN", "dialog-password"},
        {"Deposit", "go-up"},
        {"Deactivate", "process-stop"},
        {"Info", "dialog-information"},
        {"QR Code", "insert-image"}};

    int col = 0;
    for (const auto &[label, icon] : actions)
    {
        auto action_box = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL);
        action_box->set_spacing(3);
        action_box->set_halign(Gtk::ALIGN_CENTER);

        auto icon_img = Gtk::make_managed<Gtk::Image>();
        icon_img->set_from_icon_name(icon, Gtk::ICON_SIZE_MENU);

        auto action_label = Gtk::make_managed<Gtk::Label>(label);
        action_label->set_halign(Gtk::ALIGN_CENTER);
        action_label->set_margin_top(2);

        action_box->pack_start(*icon_img, Gtk::PACK_SHRINK);
        action_box->pack_start(*action_label, Gtk::PACK_SHRINK);

        auto btn = Gtk::make_managed<Gtk::Button>();
        btn->set_name("wallet-action-btn");
        btn->add(*action_box);
        btn->set_size_request(100, 60);
        grid->attach(*btn, col++, 0, 1, 1);
    }

    actions_frame->add(*grid);
    content_box->pack_start(*actions_frame, Gtk::PACK_SHRINK);

    main_box->pack_start(*content_box);
    white_frame->add(*main_box);
    outer_wrapper->pack_start(*white_frame, Gtk::PACK_SHRINK);

    return outer_wrapper;
}
