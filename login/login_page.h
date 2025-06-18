// login_page.h

#ifndef LOGIN_PAGE_H
#define LOGIN_PAGE_H

#include <gtkmm.h>

Gtk::Widget *create_login_page(Gtk::Window &window);
void show_toast_success(Gtk::Window &parent, const Glib::ustring &message);
void show_toast_fail(Gtk::Window &parent, const Glib::ustring &message);

#endif