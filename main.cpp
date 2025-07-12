#include <gtkmm.h>       // For Gui
#include "welcome.h"     // Welcome Page
#include <iostream>      // for cout, endl etc.
#include <cstdlib>       // for std::system
#include <thread>        // for std::this_thread
#include <chrono>        // for std::chrono::seconds
#include "discord_rpc.h" // Discord activity
#include "discord-activity/discord_integration.h"
#include "env.hpp"  // For env
#include <unistd.h> // for chdir()

using namespace std;

// Helper function to center window on screen
pair<int, int> getCoordinates()
{
    auto display = Gdk::Display::get_default();
    if (display)
    {
        auto monitor = display->get_primary_monitor();
        if (monitor)
        {
            Gdk::Rectangle geometry;
            monitor->get_geometry(geometry);
            int x = geometry.get_height() / 2;
            int y = geometry.get_width() / 2;
            return make_pair(x, y);
        }
    }
    return make_pair(400, 300);
}

// Updating Discord, to see User State.
bool update_discord()
{
    Discord_RunCallbacks();
    return true; // keep repeating
}

// Show Success Function
void show_toast_success(Gtk::Window &parent, const Glib::ustring &message)
{
    auto toast = new Gtk::Window(Gtk::WINDOW_POPUP);
    toast->set_decorated(false);
    toast->set_resizable(false);
    toast->set_type_hint(Gdk::WINDOW_TYPE_HINT_TOOLTIP);

    // background transparent
    Gdk::RGBA green_bg;
    green_bg.set_rgba(0.831, 0.929, 0.855, 1.0); // same thing
    toast->override_background_color(green_bg);

    int toast_width = 170;
    int margin = 40;

    // Create a container box to wrap the label
    auto box = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL);
    box->set_margin_top(20);
    box->set_margin_bottom(20);
    box->set_margin_left(10);
    box->set_margin_right(10);
    box->set_size_request(toast_width, -1);
    box->get_style_context()->add_class("toast-box");

    // 🟢 Tell GTK to expand the box fully
    box->set_hexpand(true);
    box->set_vexpand(true);

    // Create the label and style it
    auto label = Gtk::make_managed<Gtk::Label>(message);
    label->get_style_context()->add_class("toast-label");

    box->pack_start(*label, Gtk::PACK_SHRINK);
    toast->add(*box);
    toast->show_all();

    // Get parent window position
    int win_x, win_y;
    parent.get_position(win_x, win_y);

    int win_width, win_height;
    parent.get_size(win_width, win_height);

    // Place the toast in the top-right corner of the parent window
    int x = win_x + win_width - toast_width - margin;
    int y = win_y + margin;

    toast->move(x, y);

    Glib::signal_timeout().connect_once([toast]()
                                        {
        toast->hide();
        delete toast; }, 2 * 1000);
}

// Show Fail Function
void show_toast_fail(Gtk::Window &parent, const Glib::ustring &message)
{
    auto toast = new Gtk::Window(Gtk::WINDOW_POPUP);
    toast->set_decorated(false);
    toast->set_resizable(false);
    toast->set_type_hint(Gdk::WINDOW_TYPE_HINT_TOOLTIP);

    // background transparent
    Gdk::RGBA red_bg;
    red_bg.set_rgba(0.973, 0.843, 0.855, 1.0); // #f8d7da
    toast->override_background_color(red_bg);

    int toast_width = 170;
    int margin = 40;

    // Create a container box to wrap the label
    auto box = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL);
    box->set_margin_top(20);
    box->set_margin_bottom(20);
    box->set_margin_left(10);
    box->set_margin_right(10);
    box->set_size_request(toast_width, -1);
    box->get_style_context()->add_class("toast-box-fail");

    // Tell GTK to expand the box fully
    box->set_hexpand(true);
    box->set_vexpand(true);

    // Create the label and style it
    auto label = Gtk::make_managed<Gtk::Label>(message);
    label->get_style_context()->add_class("toast-label-fail");

    box->pack_start(*label, Gtk::PACK_SHRINK);
    toast->add(*box);
    toast->show_all();

    // Get parent window position
    int win_x, win_y;
    parent.get_position(win_x, win_y);

    int win_width, win_height;
    parent.get_size(win_width, win_height);

    // Place the toast in the top-right corner of the parent window
    int x = win_x + win_width - toast_width - margin;
    int y = win_y + margin;

    toast->move(x, y);

    Glib::signal_timeout().connect_once([toast]()
                                        {
        toast->hide();
        delete toast; }, 2 * 1000);
}

int main(int argc, char *argv[])
{
    chdir(".."); // Move from build/ to the project root

    cout << "Program started! " << endl;

    // Starting Node.js

    cout << "Starting Node.js backend..." << endl;
    system("node --loader ts-node/esm backend-node/2fa/Create/server.ts &");
    system("node --loader ts-node/esm backend-node/2fa/2fa-login/server.ts &");
    system("node --loader ts-node/esm backend-node/2fa/Check/server.ts &");

    // Give a server a second to boot.
    this_thread::sleep_for(chrono::seconds(1));

    // Loading enviroment and GTK, rest of the program.
    load_env(".env");

    auto app = Gtk::Application::create(argc, argv, "com.tomfi.welcomepage");

    Gtk::Window window;
    window.set_title("TomFi");

    pair<int, int> coords = getCoordinates();
    window.set_default_size(coords.second, coords.first);
    window.override_background_color(Gdk::RGBA("aliceblue")); // light background

    // Load CSS
    auto css = Gtk::CssProvider::create();
    css->load_from_path("style.css");

    auto screen = Gdk::Screen::get_default();
    Gtk::StyleContext::add_provider_for_screen(screen, css, GTK_STYLE_PROVIDER_PRIORITY_USER);

    // Discord Presence

    init_discord();

    // Updating Presence every 2 seconds
    Glib::signal_timeout().connect(sigc::ptr_fun(&update_discord), 2000);

    // Add Welcome Page UI
    Gtk::Widget *welcome_page = create_welcome(window);
    window.add(*welcome_page);

    window.show_all();

    return app->run(window);

    Discord_Shutdown();
}
