#include <gtkmm.h>
#include "chart.h"
#include <webkit2/webkit2.h>
#include <jsoncpp/json/json.h>
#include <iostream>
#include <filesystem>
#include "Session/Session.hpp"
#include <string>

using namespace std;

Gtk::Widget *create_chart_webview(Gtk::Window &window)
{
    WebKitWebView *webview = WEBKIT_WEB_VIEW(webkit_web_view_new());

    // Enable developer extras (Web Inspector)
    WebKitSettings *settings = webkit_web_view_get_settings(webview);
    webkit_settings_set_enable_developer_extras(settings, TRUE);

    // Full absolute file path to chart.html
    string html_path = string(SOURCE_DIR) + "/dashboard/chart/chart.html";
    string file_uri = "file://" + html_path;

    // Build JSON from transactions
    Json::Value root;
    for (const auto &tx : current_session.transactions)
    {
        Json::Value t;
        t["category"] = tx.category;
        root["transactions"].append(t);
    }

    Json::StreamWriterBuilder writer;
    string json = Json::writeString(writer, root);

    // Load HTML
    webkit_web_view_load_uri(webview, file_uri.c_str());

    // Inject JS after load
    g_signal_connect(webview, "load-changed", G_CALLBACK(+[](WebKitWebView *web_view, WebKitLoadEvent event, gpointer user_data)
                                                         {
                                                             if (event == WEBKIT_LOAD_FINISHED)
                                                             {
                                                                 string *json_ptr = static_cast<string *>(user_data);
                                                                 string js =
                                                                     "console.log('Running injected JS');"
                                                                     "window.injectedData = " +
                                                                     *json_ptr + ";"
                                                                                 "document.dispatchEvent(new Event('DOMContentLoaded'));";

                                                                 webkit_web_view_run_javascript(web_view, js.c_str(), nullptr, nullptr, nullptr);
                                                             }
                                                         }),
                     new string(json));

    return Glib::wrap(GTK_WIDGET(webview));
}
