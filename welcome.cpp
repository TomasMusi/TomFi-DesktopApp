#include "welcome.h"
#include <gtkmm.h>
#include <iostream>
#include "login/login_page.h"
#include "register/register.h"
#include "dashboard/dashboard.h"
#include "Session/Session.hpp"

Gtk::Widget *create_welcome(Gtk::Window &window)
{
    // Outer box centered in window
    auto outer = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL);
    outer->set_valign(Gtk::ALIGN_CENTER);
    outer->set_halign(Gtk::ALIGN_CENTER);
    outer->set_spacing(20); // space between elements

    // Title label
    auto title = Gtk::make_managed<Gtk::Label>("Welcome to\nTomFi");
    title->set_justify(Gtk::JUSTIFY_CENTER);
    title->set_halign(Gtk::ALIGN_CENTER);
    title->set_margin_bottom(20);
    title->set_name("welcome-title");

    // Login button
    auto login_button = Gtk::make_managed<Gtk::Button>("Log In");
    login_button->set_size_request(200, 40);
    login_button->set_name("primary-button");
    login_button->signal_clicked().connect([&window, outer]()
                                           {
                                               window.remove();                        // remove current widget
                                               Gtk::Widget *login_ui = create_login_page(window); // call the login GUI
                                               window.add(*login_ui);
                                               window.set_title("TomFi | Login UI");
                                               login_ui->show_all(); });

    // Register button
    auto register_button = Gtk::make_managed<Gtk::Button>("Register");
    register_button->set_size_request(200, 40);
    register_button->set_name("primary-button");
    register_button->signal_clicked().connect([&window]()
                                              { 
                                            window.remove();
                                            Gtk::Widget *register_ui = create_register_page(window);
                                            window.add(*register_ui);
                                            window.set_title("TomFi | Register UI");
                                            register_ui->show_all(); });

    // Dashboard button
    auto dashboard_button = Gtk::make_managed<Gtk::Button>("Dashboard");
    dashboard_button->set_size_request(200, 40);
    dashboard_button->set_name("primary-button");
    dashboard_button->signal_clicked().connect([&window]()
                                               {
                                                   if (current_session.is_authenticated)
                                                   {
                                                       // Proceed to dashboard
                                                       window.remove();
                                                       Gtk::Widget *dashboard_ui = create_dashboard(window);
                                                       window.add(*dashboard_ui);
                                                       window.set_title("TomFi | Dashboard");
                                                       dashboard_ui->show_all();
                                                   }
                                                   else
                                                   {
                                                       //  show a dialog warning
                                                       show_toast_fail(window, "Access Denied!");
                                                   } });

    // Pack all widgets
    outer->pack_start(*title, Gtk::PACK_SHRINK);
    outer->pack_start(*login_button, Gtk::PACK_SHRINK);
    outer->pack_start(*register_button, Gtk::PACK_SHRINK);
    outer->pack_start(*dashboard_button, Gtk::PACK_SHRINK);

    return outer;
}