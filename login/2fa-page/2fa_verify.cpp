// 2fa_verify.cpp

#include <iostream>
#include <string>
#include <gtkmm.h>
#include <curl/curl.h>       // Curl for Connection with HTTP
#include <nlohmann/json.hpp> // JSON
#include "../../dashboard/dashboard.h"

using namespace std;

Gtk::Widget *create_2fa_page(Gtk::Window &window, int user_id)
{
    auto box = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL);
    box->set_valign(Gtk::ALIGN_CENTER);
    box->set_halign(Gtk::ALIGN_CENTER);
    box->set_spacing(10);

    auto label = Gtk::make_managed<Gtk::Label>("Enter your 2FA code:");
    auto code_entry = Gtk::make_managed<Gtk::Entry>();
    code_entry->set_placeholder_text("6-digit code");

    auto submit_btn = Gtk::make_managed<Gtk::Button>("Verify");

    submit_btn->signal_clicked().connect([code_entry, user_id, &window]()
                                         {
        string code = code_entry->get_text();

        CURL *curl = curl_easy_init();
        if (!curl) return;

        string response;
        struct curl_slist *headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");

        string payload = "{\"user_id\": " + to_string(user_id) + ", \"code\": \"" + code + "\"}";

        curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:3002/2fa/verify");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, +[](char *ptr, size_t size, size_t nmemb, string *data) {
            if (data) {
                data->append(ptr, size * nmemb);
                return size * nmemb;
            }
            return size_t(0);
        });
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        CURLcode res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);

        if (res == CURLE_OK) {
            try {
                auto json = nlohmann::json::parse(response);
                if (json["success"]) {
                    Gtk::MessageDialog dialog(window, "2FA Verified!", false, Gtk::MESSAGE_INFO);
                    dialog.run();

                    window.remove();
                    Gtk::Widget *dashboard_ui = create_dashboard(window);
                    window.add(*dashboard_ui);
                    window.set_title("TomFi | Dashboard");
                    dashboard_ui->show_all();
                } else {
                    Gtk::MessageDialog dialog(window, "Invalid code. Try again.", false, Gtk::MESSAGE_ERROR);
                    dialog.run();
                }
            } catch (...) {
                Gtk::MessageDialog dialog(window, "Error verifying 2FA.", false, Gtk::MESSAGE_ERROR);
                dialog.run();
            }
        } });

    box->pack_start(*label, Gtk::PACK_SHRINK);
    box->pack_start(*code_entry, Gtk::PACK_SHRINK);
    box->pack_start(*submit_btn, Gtk::PACK_SHRINK);

    return box;
}
