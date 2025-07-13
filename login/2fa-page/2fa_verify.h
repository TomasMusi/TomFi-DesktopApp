// 2fa_verify.h

#ifndef TWOFA_PAGE_H
#define TWOFA_PAGE_H

#include <gtkmm.h>

Gtk::Widget *create_2fa_page(Gtk::Window &window, int user_id);

#endif