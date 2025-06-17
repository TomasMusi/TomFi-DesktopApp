#include <gtkmm.h>
#include "welcome.h"
#include <iostream>
#include "discord_rpc.h"
#include "discord-activity/discord_integration.h"
#include "env.hpp"

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

bool update_discord()
{
    Discord_RunCallbacks();
    return true; // keep repeating
}

int main(int argc, char *argv[])
{

    cout << "Program started! " << endl;
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
