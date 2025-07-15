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

void show_transfer_dialog(Gtk::Window &window)
{
    Gtk::Dialog dialog("Select Payment Method", window, true);
    dialog.set_default_size(500, 300);
    dialog.set_border_width(20);
    dialog.set_resizable(false);

    auto content = dialog.get_content_area();
    auto vcenter_wrapper = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL);
    vcenter_wrapper->set_vexpand(true);

    auto outer = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL);
    outer->set_spacing(16);
    outer->set_halign(Gtk::ALIGN_CENTER);
    outer->set_valign(Gtk::ALIGN_CENTER);

    // --- Method selection buttons ---
    auto options_box = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL);
    options_box->set_spacing(12);
    options_box->set_halign(Gtk::ALIGN_CENTER);
    options_box->set_hexpand(true);

    auto card_btn = Gtk::make_managed<Gtk::ToggleButton>("Card");
    card_btn->set_size_request(140, 60);
    card_btn->set_name("card-toggle");

    auto qr_btn = Gtk::make_managed<Gtk::ToggleButton>("QR Code");
    qr_btn->set_size_request(140, 60);
    qr_btn->set_name("qr-toggle");

    options_box->pack_start(*card_btn, Gtk::PACK_SHRINK);
    options_box->pack_start(*qr_btn, Gtk::PACK_SHRINK);
    outer->pack_start(*options_box, Gtk::PACK_SHRINK);

    // --- Stack wrapper ---
    auto stack_wrapper = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL);
    stack_wrapper->set_visible(false);

    auto stack = Gtk::make_managed<Gtk::Stack>();
    stack->set_transition_type(Gtk::STACK_TRANSITION_TYPE_SLIDE_LEFT_RIGHT);
    stack->set_margin_top(10);

    // --- Card Form ---
    auto card_form = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL);
    card_form->set_spacing(12);

    auto make_labeled_field = [](const string &label_text, Gtk::Entry *entry_widget, const string &id)
    {
        auto container = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL);
        auto label = Gtk::make_managed<Gtk::Label>(label_text);
        label->set_halign(Gtk::ALIGN_START);
        label->set_name(id + "-label");

        entry_widget->set_name(id);

        container->pack_start(*label, Gtk::PACK_SHRINK);
        container->pack_start(*entry_widget, Gtk::PACK_SHRINK);
        return container;
    };

    auto card_number = Gtk::make_managed<Gtk::Entry>();
    card_number->set_placeholder_text("1234 5678 9012 3456");
    card_form->pack_start(*make_labeled_field("Card Number", card_number, "card_number"), Gtk::PACK_SHRINK);

    auto username = Gtk::make_managed<Gtk::Entry>();
    username->set_placeholder_text("johndoe123");
    card_form->pack_start(*make_labeled_field("Recipient Username", username, "username"), Gtk::PACK_SHRINK);

    auto pin = Gtk::make_managed<Gtk::Entry>();
    pin->set_placeholder_text("PIN");
    pin->set_visibility(false);
    card_form->pack_start(*make_labeled_field("PIN", pin, "pin"), Gtk::PACK_SHRINK);

    auto amount = Gtk::make_managed<Gtk::Entry>();
    amount->set_placeholder_text("$0.00");
    card_form->pack_start(*make_labeled_field("Amount", amount, "amount"), Gtk::PACK_SHRINK);

    // --- Categories ---
    auto category_label = Gtk::make_managed<Gtk::Label>("Category");
    category_label->set_name("category-label");
    category_label->set_halign(Gtk::ALIGN_START);

    auto categories_box = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL);
    categories_box->set_spacing(8);
    categories_box->set_name("categories-box");

    const vector<string> category_names = {"Friend", "Food", "Sports", "Other"};
    vector<Gtk::CheckButton *> category_buttons;

    for (const auto &name : category_names)
    {
        auto check = Gtk::make_managed<Gtk::CheckButton>(name);
        check->set_name("category-" + name);
        categories_box->pack_start(*check, Gtk::PACK_SHRINK);
        category_buttons.push_back(check);
    }

    card_form->pack_start(*category_label, Gtk::PACK_SHRINK);
    card_form->pack_start(*categories_box, Gtk::PACK_SHRINK);

    // --- Message field ---
    auto message_label = Gtk::make_managed<Gtk::Label>("Message (optional)");
    message_label->set_name("message-label");
    message_label->set_halign(Gtk::ALIGN_START);

    auto message_area = Gtk::make_managed<Gtk::TextView>();
    message_area->set_wrap_mode(Gtk::WRAP_WORD);
    message_area->set_size_request(400, 60);
    message_area->set_editable(true);
    message_area->set_name("message");

    card_form->pack_start(*message_label, Gtk::PACK_SHRINK);
    card_form->pack_start(*message_area, Gtk::PACK_SHRINK);

    stack->add(*card_form, "card");

    // --- QR Form ---
    auto qr_form = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL);
    qr_form->set_spacing(8);

    auto qr_label = Gtk::make_managed<Gtk::Label>("Upload QR Code:");
    auto qr_file_chooser = Gtk::make_managed<Gtk::FileChooserButton>("Select QR Code", Gtk::FILE_CHOOSER_ACTION_OPEN);
    qr_file_chooser->set_name("qr-file");

    auto filter = Gtk::FileFilter::create();
    filter->set_name("Images");
    filter->add_mime_type("image/png");
    filter->add_mime_type("image/jpeg");
    qr_file_chooser->set_filter(filter);

    qr_form->pack_start(*qr_label, Gtk::PACK_SHRINK);
    qr_form->pack_start(*qr_file_chooser, Gtk::PACK_SHRINK);
    stack->add(*qr_form, "qr");

    stack_wrapper->pack_start(*stack, Gtk::PACK_SHRINK);
    outer->pack_start(*stack_wrapper, Gtk::PACK_SHRINK);

    // --- Logic ---
    card_btn->signal_toggled().connect([=]()
                                       {
        if (card_btn->get_active()) {
            qr_btn->set_active(false);
            stack->set_visible_child(*card_form);
            stack_wrapper->show();
        } else if (!qr_btn->get_active()) {
            stack_wrapper->hide();
        } });

    qr_btn->signal_toggled().connect([=]()
                                     {
        if (qr_btn->get_active()) {
            card_btn->set_active(false);
            stack->set_visible_child(*qr_form);
            stack_wrapper->show();
        } else if (!card_btn->get_active()) {
            stack_wrapper->hide();
        } });

    // --- Buttons ---
    dialog.add_button("Cancel", Gtk::RESPONSE_CANCEL);
    dialog.add_button("Continue", Gtk::RESPONSE_OK);

    vcenter_wrapper->pack_start(*outer, Gtk::PACK_EXPAND_WIDGET);
    content->pack_start(*vcenter_wrapper, Gtk::PACK_EXPAND_WIDGET);

    dialog.show_all();
    stack_wrapper->hide();

    if (dialog.run() == Gtk::RESPONSE_OK)
    {

        // We make sure, this only works, when we have opened the card_btn!
        if (card_btn->get_active())
        {
            string raw_card = card_number->get_text(); // it makes number like "1234 4546 7895 1234" this number: "1234454678951234" which we want to.
            raw_card.erase(remove(raw_card.begin(), raw_card.end(), ' '), raw_card.end());
            const string to = username->get_text();
            const string pin_text = pin->get_text();
            const string amount_text = amount->get_text();
            const string message = message_area->get_buffer()->get_text();

            // Validate card number
            if (raw_card.length() != 16 || !all_of(raw_card.begin(), raw_card.end(), ::isdigit)) // makes sure it is long 16 characters, and are digits!
            {
                show_toast_fail(window, "Card number must be exactly 16 digits (excluding spaces).");
                return;
            }

            // Validate PIN
            if (pin_text.length() != 4 || !all_of(pin_text.begin(), pin_text.end(), ::isdigit)) // makes sure the pin must be 4 digits long and it must be digits!
            {
                show_toast_fail(window, "PIN must be exactly 4 digits.");
                return;
            }

            // Validate username
            if (to.empty()) // if user gives and empty username, atlest 1 character is needed.
            {
                show_toast_fail(window, "Recipient username cannot be empty.");
                return;
            }

            // Validate amount
            try
            {
                double amt = stod(amount_text);
                if (amt < 1.0) // if the amount is less than 1 $
                {
                    show_toast_fail(window, "Amount must be at least $1.");
                    return;
                }
            }
            catch (...)
            { // if there is some invalid format.
                show_toast_fail(window, "Invalid amount format.");
                return;
            }

            // Validate categories
            int selected = 0;
            for (auto *check : category_buttons)
            {
                if (check->get_active())
                    selected++;
            }

            if (selected != 1)
            {
                show_toast_fail(window, "Please select exactly one category.");
                return;
            }

            // All passed, we get here only if everything above is good.
            cout << "Card payment submitted!" << endl;
            cout << "Card: " << card << endl;
            cout << "To: " << to << endl;
            cout << "Message: " << message << endl;
        }

        // this only works for qr_btn, when we have it opened.
        else if (qr_btn->get_active())
        {
            cout << "QR payment submitted!" << endl;
            cout << "QR Path: " << qr_file_chooser->get_filename() << endl;
        }
    }
}

void show_pin_dialog(Gtk::Window *parent_window)
{
    Gtk::Dialog dialog("Enter Your Password to View PIN", *parent_window, true);
    dialog.set_default_size(400, 150);
    dialog.set_border_width(20);
    dialog.set_modal(true);
    dialog.set_resizable(false);
    dialog.get_content_area()->set_spacing(10);

    auto label = Gtk::make_managed<Gtk::Label>("Please enter your account password:");
    label->set_halign(Gtk::ALIGN_START);

    auto entry = Gtk::make_managed<Gtk::Entry>();
    entry->set_visibility(false); // hide the password characters
    entry->set_placeholder_text("Your password");
    entry->set_hexpand(true);
    entry->set_margin_bottom(10);

    dialog.add_button("Cancel", Gtk::RESPONSE_CANCEL);
    dialog.add_button("View PIN", Gtk::RESPONSE_OK);

    auto box = dialog.get_content_area();
    box->pack_start(*label, Gtk::PACK_SHRINK);
    box->pack_start(*entry, Gtk::PACK_SHRINK);

    dialog.show_all();

    int result = dialog.run();
    if (result == Gtk::RESPONSE_OK)
    {
        string user_password = entry->get_text();

        string decrypted_pin = get_decrypted_pin(user_password, current_session.user_id);

        if (!decrypted_pin.empty())
        {
            Gtk::MessageDialog pin_dialog(*parent_window, "Your PIN is:", false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK, true);
            pin_dialog.set_secondary_text(decrypted_pin);
            pin_dialog.run();
        }
        else
        {
            Gtk::MessageDialog error_dialog(*parent_window, "Password incorrect or failed to decrypt PIN.", false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
            error_dialog.run();
        }
    }
}

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

        if (label == "Show PIN")
        {
            btn->signal_clicked().connect([&window]
                                          { show_pin_dialog(&window); });
        }

        if (label == "Transfer")
        {
            btn->signal_clicked().connect([&window]
                                          { show_transfer_dialog(window); });
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
