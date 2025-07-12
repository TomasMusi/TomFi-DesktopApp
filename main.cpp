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
#include <vector>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

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

// Fork, starting node servers, killing ,,Zombies" Linux.

vector<pid_t> child_pids; // Keeps track of the PIDs (Process IDs)

void start_node_server(const char *relative_script_path)
{
    pid_t pid = fork(); // Creates a new Process. It clones the current process. The return vallues tells you where you are. Example: pid == 0 (You're in the child process.) pid > 0 (you're in the parent+ pid is the child's PID). pid < 0 (Fork failed)

    // if in child process
    if (pid == 0)
    {
        // Child process
        if (chdir("backend-node") != 0)
        {
            perror("Failed to chdir to backend-node");
            exit(1);
        }
        // AFTER THIS LINE, THE CHILD BECOMES THE NODE.JS PROCESS.
        execlp("node", "node", "--loader", "ts-node/esm", relative_script_path, (char *)NULL);
        perror("Failed to start Node.js");
        exit(1);
    }
    else if (pid > 0)
    {
        child_pids.push_back(pid);
        cout << "Started Node.js server: backend-node/" << relative_script_path << " (PID " << pid << ")" << endl;
    }
    else
    {
        perror("Fork failed");
    }
}

void stop_all_node_servers()
{
    for (pid_t pid : child_pids)
    {
        kill(pid, SIGTERM);    // Graceful ask process to terminate
        waitpid(pid, NULL, 0); // Reap the child (removes zombies)
        cout << "Stopped Node.js server (PID " << pid << ")" << endl;
    }
    child_pids.clear();
}

// ensure we don't leave zombies if anything crashes
void handle_sigchld(int)
{
    while (waitpid(-1, NULL, WNOHANG) > 0)
    {
    }
}

int main(int argc, char *argv[])
{
    chdir(".."); // Move from build/ to the project root

    cout << "Program started! " << endl;

    // Reap zombies automatically
    signal(SIGCHLD, handle_sigchld);

    cout << "Starting Node.js backend..." << endl;
    start_node_server("2fa/Create/server.ts");
    start_node_server("2fa/2fa-login/server.ts");
    start_node_server("2fa/Check/server.ts");

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

    // Closing App
    app->signal_shutdown().connect([]
                                   {
        stop_all_node_servers();   // ensures server shutdown even from UI close
        Discord_Shutdown(); });

    int status = app->run(window);
    return status;
}
