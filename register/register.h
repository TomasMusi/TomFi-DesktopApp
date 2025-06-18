// register.h

#ifndef REGISTER_PAGE_H
#define REGISTER_PAGE_H

#include <gtkmm.h>

Gtk::Widget *create_register_page(Gtk::Window &window);
void show_toast_success(Gtk::Window &parent, const Glib::ustring &message);
void show_toast_fail(Gtk::Window &parent, const Glib::ustring &message);

#endif