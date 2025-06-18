// welcome.h

#ifndef WELCOME_H
#define WELCOME_H

#include <gtkmm.h>

Gtk::Widget *create_welcome(Gtk::Window &window);
void show_toast_success(Gtk::Window &parent, const Glib::ustring &message);
void show_toast_fail(Gtk::Window &parent, const Glib::ustring &message);
#endif