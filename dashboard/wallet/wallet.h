// Wallet.h

#ifndef WALLET_PAGE_H
#define WALLET_PAGE_H

#include <gtkmm.h>

Gtk::Widget *create_wallet_ui(Gtk::Window &window);
void show_toast_success(Gtk::Window &parent, const Glib::ustring &message);
void show_toast_fail(Gtk::Window &parent, const Glib::ustring &message);

#endif