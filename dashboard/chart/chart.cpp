#include <gtkmm.h>
#include "chart.h"
#include <webkit2/webkit2.h>
#include <jsoncpp/json/json.h>
#include <fstream>
#include <sstream>
#include <filesystem>
#include "Session/Session.hpp"
#include <string>

using namespace std;

Gtk::Widget *create_chart_webview(Gtk::Window &window)
{
    WebKitWebView *webview = WEBKIT_WEB_VIEW(webkit_web_view_new());

    // Full absolute file path to chart.html
    string html_path = std::filesystem::absolute("dashboard/chart/chart.html").string();
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

    // Load HTML file from file://
    webkit_web_view_load_uri(webview, file_uri.c_str());

    // Inject JavaScript after page is loaded
    g_signal_connect(webview, "load-changed", G_CALLBACK(+[](WebKitWebView *web_view, WebKitLoadEvent event, gpointer user_data)
                                                         {
                                                             if (event == WEBKIT_LOAD_FINISHED)
                                                             {
                                                                 string *json_ptr = static_cast<string *>(user_data);
                                                                 string js = "window.injectedData = " + *json_ptr + "; document.dispatchEvent(new Event('DOMContentLoaded'));";
                                                                 webkit_web_view_run_javascript(web_view, js.c_str(), nullptr, nullptr, nullptr);
                                                             }
                                                         }),
                     new string(json));     // optional: handle deletion
    return Glib::wrap(GTK_WIDGET(webview)); // converts to C++ GTK: Widget
}
