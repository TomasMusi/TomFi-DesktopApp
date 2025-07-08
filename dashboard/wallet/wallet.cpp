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
#include "db/database.h"

using namespace std;

void show_deposit_dialog(Gtk::Window *parent_window)
{
    Gtk::Dialog dialog("Add Funds to Your Account", *parent_window, true);
    dialog.set_default_size(400, 200);
    dialog.set_border_width(20);
    dialog.set_modal(true);
    dialog.set_resizable(false);
    dialog.get_content_area()->set_spacing(10);

    // Entry field
    auto label = Gtk::make_managed<Gtk::Label>("Amount (USD)");
    label->set_halign(Gtk::ALIGN_START);

    auto entry = Gtk::make_managed<Gtk::Entry>();
    entry->set_placeholder_text("e.g. 100");
    entry->set_input_purpose(Gtk::INPUT_PURPOSE_NUMBER);
    entry->set_hexpand(true);
    entry->set_margin_bottom(10);

    // Buttons
    dialog.add_button("Cancel", Gtk::RESPONSE_CANCEL);
    dialog.add_button("Append", Gtk::RESPONSE_OK);

    // Layout
    auto box = dialog.get_content_area();
    box->pack_start(*label, Gtk::PACK_SHRINK);
    box->pack_start(*entry, Gtk::PACK_SHRINK);

    dialog.show_all();

    // Response handling
    int result = dialog.run();
    if (result == Gtk::RESPONSE_OK)
    {
        string amount_str = entry->get_text();
        try
        {
            int amount = std::stoi(amount_str);
            if (amount >= 1)
            {
                cout << "Append funds: " << amount << std::endl;
                add_funds_balance(current_session.user_id, amount);
                Gtk::Widget *new_ui = create_wallet_ui(*parent_window);
                parent_window->remove();
                parent_window->add(*new_ui);
                parent_window->set_title("TomFi | Wallet");
                new_ui->show_all();
            }
            else
            {
                Gtk::MessageDialog error_dialog(*parent_window, "Please enter a number greater than or equal to 1.", false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
                error_dialog.run();
            }
        }
        catch (const exception &e)
        {
            Gtk::MessageDialog error_dialog(*parent_window, "Invalid input. Please enter a valid integer.", false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
            error_dialog.run();
        }
    }
}

void card_status(Gtk::Window &window, int user_id)
{
    if (card_status_toggle(user_id))
    {
        show_toast_success(window, current_session.is_active ? "✅ Card activated" : "❌ Card deactivated");
        //  Refresh Wallet UI after change:
        Gtk::Widget *new_ui = create_wallet_ui(window);
        window.remove();
        window.add(*new_ui);
        window.set_title("TomFi | Wallet");
        new_ui->show_all();
    }
    else
    {
        show_toast_fail(window, "Failed to update card status.");
    }
}

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

    // Credit Card
    auto card_frame = Gtk::make_managed<Gtk::Frame>();
    card_frame->set_name("credit-card-frame");
    card_frame->set_shadow_type(Gtk::SHADOW_NONE);

    // Main vertical container
    auto card_box = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL);
    card_box->set_spacing(10);
    card_box->set_margin_top(10);
    card_box->set_margin_bottom(10);
    card_box->set_margin_start(15);
    card_box->set_margin_end(15);

    // Top row: balance + active badge
    auto top_row = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL);
    auto balance = Gtk::make_managed<Gtk::Label>(card_balance);
    balance->set_name("card-balance");
    balance->set_halign(Gtk::ALIGN_START);
    balance->set_hexpand(true);

    auto active_badge = Gtk::make_managed<Gtk::Label>(
        current_session.is_active ? "✅ Active" : "❌ Deactivated");
    active_badge->set_name("active-badge");
    active_badge->set_halign(Gtk::ALIGN_END);
    active_badge->set_margin_top(3);

    if (current_session.is_active)
        active_badge->get_style_context()->add_class("active-badge");
    else
        active_badge->get_style_context()->add_class("inactive-badge");

    top_row->pack_start(*balance, Gtk::PACK_EXPAND_WIDGET);
    top_row->pack_start(*active_badge, Gtk::PACK_SHRINK);
    card_box->pack_start(*top_row, Gtk::PACK_SHRINK);

    // Center card number
    auto card_number_label = Gtk::make_managed<Gtk::Label>(card_number);
    card_number_label->set_halign(Gtk::ALIGN_START);
    card_number_label->set_hexpand(true);
    card_box->pack_start(*card_number_label, Gtk::PACK_SHRINK);

    // Info row: Cardholder + Expiry/CVV
    auto info_row = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL);
    info_row->set_spacing(10);
    info_row->set_hexpand(true);

    // Cardholder (left column)
    auto cardholder_box = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL);

    // Title: "Cardholder"
    auto cardholder_title = Gtk::make_managed<Gtk::Label>("Cardholder");
    cardholder_title->set_name("card-info-label");
    cardholder_title->set_halign(Gtk::ALIGN_START);

    // Value: username in bold
    auto cardholder_name = Gtk::make_managed<Gtk::Label>();
    cardholder_name->set_markup("<b>" + current_session.name + "</b>");
    cardholder_name->set_halign(Gtk::ALIGN_START);

    cardholder_box->pack_start(*cardholder_title, Gtk::PACK_SHRINK);
    cardholder_box->pack_start(*cardholder_name, Gtk::PACK_SHRINK);
    cardholder_box->set_hexpand(true);
    cardholder_box->set_halign(Gtk::ALIGN_START);

    // Expiry/CVV (right column)
    auto exp_cvv_box = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL);
    auto labels_row = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL);
    auto values_row = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL);

    auto expiry_label = Gtk::make_managed<Gtk::Label>("EXPIRES");
    auto cvv_label = Gtk::make_managed<Gtk::Label>("CVV");
    expiry_label->set_halign(Gtk::ALIGN_END);
    cvv_label->set_halign(Gtk::ALIGN_END);
    expiry_label->set_margin_end(20);

    labels_row->pack_start(*expiry_label, Gtk::PACK_EXPAND_WIDGET);
    labels_row->pack_start(*cvv_label, Gtk::PACK_EXPAND_WIDGET);

    auto expiry_value = Gtk::make_managed<Gtk::Label>("12/24");
    auto cvv_value = Gtk::make_managed<Gtk::Label>("***");
    expiry_value->set_halign(Gtk::ALIGN_END);
    cvv_value->set_halign(Gtk::ALIGN_END);
    expiry_value->set_margin_end(20);

    values_row->pack_start(*expiry_value, Gtk::PACK_EXPAND_WIDGET);
    values_row->pack_start(*cvv_value, Gtk::PACK_EXPAND_WIDGET);

    exp_cvv_box->pack_start(*labels_row, Gtk::PACK_SHRINK);
    exp_cvv_box->pack_start(*values_row, Gtk::PACK_SHRINK);
    exp_cvv_box->set_halign(Gtk::ALIGN_END);
    exp_cvv_box->set_hexpand(true);

    // Add to info row
    info_row->pack_start(*cardholder_box, Gtk::PACK_EXPAND_WIDGET);
    info_row->pack_start(*exp_cvv_box, Gtk::PACK_EXPAND_WIDGET);
    card_box->pack_start(*info_row, Gtk::PACK_SHRINK);

    // Bottom row: Button aligned right
    auto btn_row = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL);
    btn_row->set_hexpand(true);
    auto add_btn = Gtk::make_managed<Gtk::Button>("+ New transaction");
    add_btn->set_name("add-transaction");
    add_btn->set_halign(Gtk::ALIGN_END);
    btn_row->pack_end(*add_btn, Gtk::PACK_SHRINK);
    card_box->pack_start(*btn_row, Gtk::PACK_SHRINK);

    // Final assembly
    card_frame->add(*card_box);
    content_box->pack_start(*card_frame, Gtk::PACK_SHRINK);

    // Action Wrapper
    auto actions_wrapper = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL);
    actions_wrapper->set_name("wallet-actions-wrapper");
    actions_wrapper->set_margin_top(20);
    actions_wrapper->set_margin_bottom(20);

    // Grid
    auto grid = Gtk::make_managed<Gtk::Grid>();
    grid->set_column_spacing(16);
    grid->set_row_spacing(16);
    grid->set_valign(Gtk::ALIGN_CENTER);
    grid->set_halign(Gtk::ALIGN_CENTER);

    vector<tuple<string, string, string>> actions = {
        {"Transfer", "system-run", "action-icon-blue"},
        {"Show PIN", "dialog-password", "action-icon-indigo"},
        {"Deposit", "go-up", "action-icon-green"},
        {current_session.is_active ? "Deactivate" : "Activate",
         current_session.is_active ? "media-playback-stop" : "media-playback-start",
         "action-icon-red"},
        {"Info", "dialog-information", "action-icon-yellow"},
        {"QR Code", "insert-image", "action-icon-purple"}};

    int col = 0;
    for (const auto &[label, icon, icon_class] : actions)
    {
        auto action_box = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL);
        action_box->set_spacing(5);
        action_box->set_halign(Gtk::ALIGN_CENTER);

        auto icon_img = Gtk::make_managed<Gtk::Image>();
        icon_img->set_from_icon_name(icon, Gtk::ICON_SIZE_DIALOG);
        icon_img->get_style_context()->add_class(icon_class);

        auto action_label = Gtk::make_managed<Gtk::Label>(label);
        action_label->set_name("wallet-action-label");
        action_label->set_halign(Gtk::ALIGN_CENTER);

        action_box->pack_start(*icon_img, Gtk::PACK_SHRINK);
        action_box->pack_start(*action_label, Gtk::PACK_SHRINK);

        auto btn = Gtk::make_managed<Gtk::Button>();
        btn->set_name("wallet-action-btn");
        btn->add(*action_box);
        btn->set_size_request(110, 80);
        btn->set_margin_start(8);
        btn->set_margin_end(8);
        btn->set_margin_top(8);
        btn->set_margin_bottom(8);

        if (label == "Deposit")
        {
            btn->signal_clicked().connect([&window]
                                          {
                                              show_deposit_dialog(&window); // <- Pass the parent window
                                          });
        }

        if (label == "Deactivate" || label == "Activate")
        {
            btn->signal_clicked().connect([&window]
                                          { card_status(window, current_session.user_id); });
        }

        grid->attach(*btn, col, 0, 1, 1);
        grid->set_column_homogeneous(true);
        col++;
    }

    actions_wrapper->pack_start(*grid, Gtk::PACK_SHRINK);
    content_box->pack_start(*actions_wrapper, Gtk::PACK_SHRINK);

    main_box->pack_start(*content_box);
    white_frame->add(*main_box);
    outer_wrapper->pack_start(*white_frame, Gtk::PACK_SHRINK);

    return outer_wrapper;
}
