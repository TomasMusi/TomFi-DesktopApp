#include <gtkmm.h>
#include <iostream>
#include <string>
#include "Session/Session.hpp"
#include <vector>
#include "transaction.h"
#include "welcome.h"
#include "dashboard/dashboard.h"

using namespace std;

vector<Transaction> fetch_transactions()
{
    return {
        {"June 27, 2025", "deposit", "other", "541.00", "in"},
        {"June 27, 2025", "Pizza", "Friend", "150.00", "out"},
    };
}

Gtk::Widget *create_transactions_ui(Gtk::Window &window)
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

    auto transactions_btn = Gtk::make_managed<Gtk::Button>("Transactions");
    transactions_btn->set_name("side-button-current");
    transactions_btn->set_halign(Gtk::ALIGN_START);
    sidebar->pack_start(*transactions_btn, Gtk::PACK_SHRINK);

    auto mail_btn = Gtk::make_managed<Gtk::Button>("Mail");
    mail_btn->set_name("sidebar-button");
    mail_btn->set_halign(Gtk::ALIGN_START);
    sidebar->pack_start(*mail_btn, Gtk::PACK_SHRINK);

    auto wallet_btn = Gtk::make_managed<Gtk::Button>("Wallet");
    wallet_btn->set_name("sidebar-button");
    wallet_btn->set_halign(Gtk::ALIGN_START);
    sidebar->pack_start(*wallet_btn, Gtk::PACK_SHRINK);

    auto settings_btn = Gtk::make_managed<Gtk::Button>("Settings");
    settings_btn->set_name("sidebar-button");
    settings_btn->set_halign(Gtk::ALIGN_START);
    sidebar->pack_start(*settings_btn, Gtk::PACK_SHRINK);

    sidebar_wrapper->add(*sidebar);
    main_box->pack_start(*sidebar_wrapper, Gtk::PACK_SHRINK);

    auto content_box = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL);
    content_box->set_name("dashboard-content");
    content_box->set_spacing(20);
    content_box->set_margin_top(20);
    content_box->set_margin_start(20);
    content_box->set_margin_end(20);

    auto heading = Gtk::make_managed<Gtk::Label>("Transactions");
    heading->set_name("dashboard-title");
    heading->set_margin_bottom(10);
    content_box->pack_start(*heading, Gtk::PACK_SHRINK);

    auto table_frame = Gtk::make_managed<Gtk::Frame>();
    table_frame->set_name("transaction-table-frame");
    table_frame->set_shadow_type(Gtk::SHADOW_NONE);

    auto scrolled = Gtk::make_managed<Gtk::ScrolledWindow>();
    scrolled->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    scrolled->set_min_content_height(300);

    auto list = Gtk::make_managed<Gtk::Grid>();
    list->set_row_spacing(8);
    list->set_column_spacing(16);
    list->set_margin_top(10);
    list->set_margin_bottom(10);
    list->set_margin_start(10);
    list->set_margin_end(10);

    vector<string> headers = {"Date", "Description", "Category", "Amount"};
    for (size_t i = 0; i < headers.size(); ++i)
    {
        auto header_label = Gtk::make_managed<Gtk::Label>(headers[i]);
        header_label->set_markup("<b>" + headers[i] + "</b>");
        header_label->set_name("table-header");
        list->attach(*header_label, i, 0, 1, 1);
    }

    auto transactions = fetch_transactions();
    int row = 1;
    for (const auto &tx : transactions)
    {
        for (int col = 0; col < 4; ++col)
        {
            string text;
            string css_class = "table-cell"; // default

            if (col == 0)
                text = tx.date;
            else if (col == 1)
                text = tx.description;
            else if (col == 2)
                text = tx.category;
            else if (col == 3)
            {
                text = (tx.direction == "out" ? "-$" : "+$") + tx.amount;
                css_class = tx.direction == "out" ? "amount-out" : "amount-in";
            }

            auto cell = Gtk::make_managed<Gtk::Label>(text);
            cell->set_name(css_class);
            cell->set_hexpand(true);
            cell->set_halign(Gtk::ALIGN_FILL);
            list->attach(*cell, col, row, 1, 1);
        }
        ++row;
    }

    scrolled->add(*list);
    table_frame->add(*scrolled);
    content_box->pack_start(*table_frame, Gtk::PACK_EXPAND_WIDGET);

    main_box->pack_start(*content_box);
    white_frame->add(*main_box);
    outer_wrapper->pack_start(*white_frame, Gtk::PACK_SHRINK);

    return outer_wrapper;
}
