#include <gtkmm.h>
#include <webkit2/webkit2.h>
#include <jsoncpp/json/json.h>
#include <iostream>
#include <filesystem>
#include "Session/Session.hpp"
#include "longChart.h"

using namespace std;

Gtk::Widget *create_longchart_webview(Gtk::Window &window)
{
    WebKitWebView *webview = WEBKIT_WEB_VIEW(webkit_web_view_new());

    // Enable dev tools
    WebKitSettings *settings = webkit_web_view_get_settings(webview);
    webkit_settings_set_enable_developer_extras(settings, TRUE);

    // Full path to chart.html
    string html_path = string(SOURCE_DIR) + "/dashboard/chart/longChart.html";
    string file_uri = "file://" + html_path;

    // Build JSON from transactions
    Json::Value root;
    for (const auto &tx : current_session.transactions)
    {
        Json::Value t;
        t["timestamp"] = tx.timestamp;
        t["amount"] = tx.amount;
        t["direction"] = tx.direction;
        root["transactions"].append(t);
    }

    Json::StreamWriterBuilder writer;
    string json = Json::writeString(writer, root);

    // Load HTML
    webkit_web_view_load_uri(webview, file_uri.c_str());

    // Inject JavaScript after page load
    g_signal_connect(webview, "load-changed", G_CALLBACK(+[](WebKitWebView *web_view, WebKitLoadEvent event, gpointer user_data)
                                                         {
                                                             if (event == WEBKIT_LOAD_FINISHED)
                                                             {
                                                                 string *json_ptr = static_cast<string *>(user_data);
                                                                 string js =
                                                                     "console.log('Injecting longChart data');"
                                                                     "window.injectedData = " +
                                                                     *json_ptr + ";"
                                                                                 "document.dispatchEvent(new Event('DOMContentLoaded'));";
                                                                 webkit_web_view_run_javascript(web_view, js.c_str(), nullptr, nullptr, nullptr);
                                                             }
                                                         }),
                     new std::string(json));

    return Glib::wrap(GTK_WIDGET(webview));
}