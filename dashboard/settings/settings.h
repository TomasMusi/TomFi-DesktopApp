// settings.h

#ifndef SETTINGS_PAGE_H
#define SETTINGS_PAGE_H

#include <gtkmm.h>

Gtk::Widget *create_settings(Gtk::Window &window);
void show_toast_success(Gtk::Window &parent, const Glib::ustring &message);
void show_toast_fail(Gtk::Window &parent, const Glib::ustring &message);
#endif