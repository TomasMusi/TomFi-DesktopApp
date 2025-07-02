#include <gtkmm.h>
#include <iostream>
#include <string>
#include "dashboard.h"
#include "chart/chart.h"
#include "Session/Session.hpp"
#include "welcome.h"
#include "Transaction/transaction.h"

using namespace std;

Gtk::Widget *create_dashboard(Gtk::Window &window)
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

    string welcome_text = "Welcome " + current_session.name + "!";
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
    // Dashboard Button
    auto dashboard_btn = Gtk::make_managed<Gtk::Button>("Dashboard");
    dashboard_btn->set_name("side-button-current");
    dashboard_btn->set_halign(Gtk::ALIGN_START);
    dashboard_btn->signal_clicked().connect([&window]()
                                            {
    Gtk::Widget *dashboard_ui = create_dashboard(window);
    window.remove();
    window.add(*dashboard_ui);
    window.set_title("TomFi | Dashboard");
    dashboard_ui->show_all(); });
    sidebar->pack_start(*dashboard_btn, Gtk::PACK_SHRINK);

    // Transactions Button
    auto transactions_btn = Gtk::make_managed<Gtk::Button>("Transactions");
    transactions_btn->set_name("sidebar-button");
    transactions_btn->set_halign(Gtk::ALIGN_START);
    transactions_btn->signal_clicked().connect([&window]()
                                               {
    Gtk::Widget *transactions_ui = create_transactions_ui(window);
    window.remove();
    window.add(*transactions_ui);
    window.set_title("TomFi | Transactions");
    transactions_ui->show_all(); });
    sidebar->pack_start(*transactions_btn, Gtk::PACK_SHRINK);

    // Mail Button
    auto mail_btn = Gtk::make_managed<Gtk::Button>("Mail");
    mail_btn->set_name("sidebar-button");
    mail_btn->set_halign(Gtk::ALIGN_START);
    mail_btn->signal_clicked().connect([&window]()
                                       {
        Gtk::Widget *chart_ui = create_chart_webview(window);
        window.remove();
        window.add(*chart_ui);
        window.set_title("TomFi | Charts");
        chart_ui->show_all(); });
    // TODO: connect mail_btn->signal_clicked() to mail page if exists
    sidebar->pack_start(*mail_btn, Gtk::PACK_SHRINK);

    // Wallet Button
    auto wallet_btn = Gtk::make_managed<Gtk::Button>("Wallet");
    wallet_btn->set_name("sidebar-button");
    wallet_btn->set_halign(Gtk::ALIGN_START);
    // TODO: connect wallet_btn->signal_clicked() to wallet page
    sidebar->pack_start(*wallet_btn, Gtk::PACK_SHRINK);

    // Settings Button
    auto settings_btn = Gtk::make_managed<Gtk::Button>("Settings");
    settings_btn->set_name("sidebar-button");
    settings_btn->set_halign(Gtk::ALIGN_START);
    // TODO: connect settings_btn->signal_clicked() to settings page
    sidebar->pack_start(*settings_btn, Gtk::PACK_SHRINK);

    sidebar_wrapper->add(*sidebar);
    main_box->pack_start(*sidebar_wrapper, Gtk::PACK_SHRINK);

    auto content_box = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL);
    content_box->set_name("dashboard-content");
    content_box->set_spacing(20);
    content_box->set_margin_top(20);
    content_box->set_margin_start(20);
    content_box->set_margin_end(20);

    auto header = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL);
    auto welcome = Gtk::make_managed<Gtk::Label>(welcome_text);
    welcome->set_name("dashboard-welcome");
    header->pack_start(*welcome, Gtk::PACK_SHRINK);
    content_box->pack_start(*header, Gtk::PACK_SHRINK);

    auto top_row = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL);
    top_row->set_spacing(20);

    auto expenses_card = Gtk::make_managed<Gtk::Frame>("Expenses by category");
    expenses_card->set_name("dashboard-card");
    expenses_card->set_size_request(200, 150);
    expenses_card->set_shadow_type(Gtk::SHADOW_NONE);
    top_row->pack_start(*expenses_card, Gtk::PACK_EXPAND_WIDGET);

    auto transactions_card = Gtk::make_managed<Gtk::Frame>();
    transactions_card->set_name("dashboard-card");
    transactions_card->set_size_request(200, 150);
    transactions_card->set_shadow_type(Gtk::SHADOW_NONE);

    auto tx_wrapper = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL);
    tx_wrapper->set_spacing(6);
    tx_wrapper->set_margin_top(10);
    tx_wrapper->set_margin_start(10);
    tx_wrapper->set_margin_end(10);

    auto tx_title = Gtk::make_managed<Gtk::Label>("Transactions");
    tx_title->set_name("dashboard-section-title");
    tx_title->set_margin_bottom(6);
    tx_title->set_xalign(0);
    tx_wrapper->pack_start(*tx_title, Gtk::PACK_SHRINK);

    auto tx_list = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL);
    tx_list->set_spacing(4);

    // Giving Transaction list of 3
    int tx_count = 0;
    for (const auto &tx : current_session.transactions)
    {
        if (tx_count++ >= 3)
            break;

        auto row = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL);
        row->set_spacing(10);

        string short_desc = tx.description.length() > 25 ? tx.description.substr(0, 25) + "..." : tx.description;

        auto desc_label = Gtk::make_managed<Gtk::Label>(short_desc);
        desc_label->set_halign(Gtk::ALIGN_START);
        desc_label->set_hexpand(true);

        auto amount_label = Gtk::make_managed<Gtk::Label>((tx.direction == "out" ? "-" : "+") + tx.amount);
        amount_label->set_halign(Gtk::ALIGN_END);

        if (tx.direction == "out")
            amount_label->get_style_context()->add_class("amount-out");
        else
            amount_label->get_style_context()->add_class("amount-in");

        row->pack_start(*desc_label);
        row->pack_start(*amount_label);
        tx_list->pack_start(*row, Gtk::PACK_SHRINK);
    }

    if (current_session.transactions.empty())
    {
        auto none = Gtk::make_managed<Gtk::Label>("No transactions available");
        none->set_name("dashboard-no-tx");
        tx_list->pack_start(*none, Gtk::PACK_SHRINK);
    }

    tx_wrapper->pack_start(*tx_list, Gtk::PACK_SHRINK);
    transactions_card->add(*tx_wrapper);
    top_row->pack_start(*transactions_card, Gtk::PACK_EXPAND_WIDGET);

    // Card Section with Balance + Active
    auto card_frame = Gtk::make_managed<Gtk::Frame>();
    card_frame->set_name("credit-card-frame");
    card_frame->set_shadow_type(Gtk::SHADOW_NONE);

    auto card_box = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL);
    card_box->set_spacing(5);

    //  Balance + Active row
    auto balance_row = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL);
    balance_row->set_spacing(10);

    auto balance = Gtk::make_managed<Gtk::Label>(card_balance);
    balance->set_name("card-balance");
    balance->set_halign(Gtk::ALIGN_START);
    balance->set_hexpand(true);

    auto active_badge = Gtk::make_managed<Gtk::Label>(
        current_session.is_active ? "✅ Active" : "❌ Deactivated");
    active_badge->set_name("active-badge");
    active_badge->set_margin_top(3);
    active_badge->set_halign(Gtk::ALIGN_END);
    if (current_session.is_active)
    {
        active_badge->get_style_context()->add_class("active-badge");
    }
    else
    {
        active_badge->get_style_context()->add_class("inactive-badge");
    }
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
    top_row->pack_start(*card_frame, Gtk::PACK_EXPAND_WIDGET);

    content_box->pack_start(*top_row, Gtk::PACK_SHRINK);

    auto analytics = Gtk::make_managed<Gtk::Frame>("Analytics");
    analytics->set_name("dashboard-card");
    analytics->set_shadow_type(Gtk::SHADOW_NONE);
    analytics->set_size_request(600, 250);
    content_box->pack_start(*analytics, Gtk::PACK_SHRINK);

    main_box->pack_start(*content_box);
    white_frame->add(*main_box);
    outer_wrapper->pack_start(*white_frame, Gtk::PACK_SHRINK);

    return outer_wrapper;
}
