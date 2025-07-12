#include <gtkmm.h>
#include <string>
#include "Session/Session.hpp"
#include "settings.h"
#include "dashboard/wallet/wallet.h"
#include "dashboard/dashboard.h"
#include <curl/curl.h>       // Curl for Connection with HTTP
#include <nlohmann/json.hpp> // JSON
#include <iostream>
#include "welcome.h"
#include "db/database.h"
#include <regex>
#include <sstream>
#include <iterator>
#include <iomanip>
#include <vector>
#include <string>
#include <gtkmm/image.h>
#include <gtkmm/dialog.h>
#include <gtkmm/box.h>
#include <glibmm.h>
#include <gdkmm/pixbuf.h>

using namespace std;

// Function for decoding base 64 to raw binary
vector<unsigned char> decode_base64(const string &input)
{
    gsize len = 0;
    guchar *data = g_base64_decode(input.c_str(), &len);
    vector<unsigned char> result(data, data + len);
    g_free(data);
    return result;
}

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

    auto enable_2fa_btn = Gtk::make_managed<Gtk::Button>("Enable 2FA");
    enable_2fa_btn->set_name("enable-2fa-btn");

    auto signout_btn = Gtk::make_managed<Gtk::Button>("Sign Out");
    signout_btn->set_name("signout-btn");

    auto save_btn = Gtk::make_managed<Gtk::Button>("Save Changes");
    save_btn->set_name("save-btn");

    control_box->pack_start(*enable_2fa_btn, Gtk::PACK_SHRINK);
    control_box->pack_start(*signout_btn, Gtk::PACK_SHRINK);
    control_box->pack_end(*save_btn, Gtk::PACK_SHRINK);

    content_box->pack_start(*control_box, Gtk::PACK_SHRINK);

    signout_btn->signal_clicked().connect([&window]()
                                          {
                                              cout << "Signed out." << endl;

                                              // Reset session data
                                              current_session = Session(); // Resets all fields to default

                                              // Show logout dialog
                                              Gtk::MessageDialog dialog(window, "You have been signed out.", false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK, true);
                                              dialog.run();

                                              // Navigate back to welcome screen
                                              Gtk::Widget *welcome_screen = create_welcome(window);
                                              window.remove();
                                              window.add(*welcome_screen);
                                              window.set_title("TomFi | Welcome");
                                              welcome_screen->show_all(); });

    enable_2fa_btn->signal_clicked().connect([&window]()
                                             {
    const string &email = current_session.email;
    string json = "{\"email\": \"" + email + "\"}";

    CURL *curl = curl_easy_init();
    if (curl)
    {
        CURLcode res;
        string response_data;
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");

        curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:3000/2fa/create");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json.c_str());

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, +[](char *ptr, size_t size, size_t nmemb, string *data)
        {
            if (data)
            {
                data->append(ptr, size * nmemb);
                return size * nmemb;
            }
            return size_t(0);
        });
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_data);

        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);

        if (res == CURLE_OK)
        {
            try
            {
                auto parsed = nlohmann::json::parse(response_data);
                if (parsed["success"])
                {
                    string qr_data_url = parsed["qr"];
                    string secret = parsed["secret"];

                    const string prefix = "data:image/png;base64,";
                    if (qr_data_url.find(prefix) != 0)
                    {
                        Gtk::MessageDialog dialog(window, "Invalid QR code format.", false, Gtk::MESSAGE_ERROR);
                        dialog.run();
                        return;
                    }

                    string base64_str = qr_data_url.substr(prefix.length());
                    auto image_data = decode_base64(base64_str);

                    try
                    {
                        auto stream = Gio::MemoryInputStream::create();
                        stream->add_data(image_data.data(), image_data.size());

                        auto pixbuf = Gdk::Pixbuf::create_from_stream_at_scale(stream, 256, 256, true);

                        Gtk::Dialog qrDialog("Scan QR and Enter Code", window, true);
                        qrDialog.set_default_size(320, 400);
                        auto box = qrDialog.get_content_area();

                        // Image
                        auto image = Gtk::make_managed<Gtk::Image>(pixbuf);
                        box->pack_start(*image, Gtk::PACK_SHRINK);

                        // Code input
                        auto code_entry = Gtk::make_managed<Gtk::Entry>();
                        code_entry->set_placeholder_text("Enter 2FA Code");
                        box->pack_start(*code_entry, Gtk::PACK_SHRINK);

                        // Confirm button
                        auto confirm_btn = Gtk::make_managed<Gtk::Button>("Confirm");
                        confirm_btn->signal_clicked().connect([code_entry, &qrDialog]() {
                            string code = code_entry->get_text();
                            cout << "confirmed" << endl;
                            qrDialog.close();  // Close the dialog after confirming
                        });
                        box->pack_start(*confirm_btn, Gtk::PACK_SHRINK);

                        box->show_all();
                        qrDialog.run();

                        store_2fa_secret_to_db(current_session.user_id, secret);
                    }
                    catch (const exception &e)
                    {
                        Gtk::MessageDialog dialog(window, "Failed to display QR code.", false, Gtk::MESSAGE_ERROR);
                        dialog.set_secondary_text(e.what());
                        dialog.run();
                    }
                }
                else
                {
                    Gtk::MessageDialog dialog(window, "Failed to enable 2FA.", false, Gtk::MESSAGE_ERROR);
                    dialog.run();
                }
            }
            catch (...)
            {
                Gtk::MessageDialog dialog(window, "Invalid response from 2FA server.", false, Gtk::MESSAGE_ERROR);
                dialog.run();
            }
        }
        else
        {
            Gtk::MessageDialog dialog(window, "Cannot reach 2FA server.", false, Gtk::MESSAGE_ERROR);
            dialog.run();
        }
    } });

    auto center_wrapper = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL);
    center_wrapper->set_halign(Gtk::ALIGN_CENTER);
    center_wrapper->set_hexpand(true);

    center_wrapper->pack_start(*content_box, Gtk::PACK_SHRINK);
    main_box->pack_start(*center_wrapper, Gtk::PACK_EXPAND_WIDGET);

    white_frame->add(*main_box);
    outer_wrapper->pack_start(*white_frame);

    return outer_wrapper;
}