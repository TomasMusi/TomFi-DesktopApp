// chart.h

#ifndef CHART_WEBVIEW_H
#define CHART_WEBVIEW_H

#include <gtkmm.h>

// Returns a WebKit-based widget with Chart.js loaded and user data injected
Gtk::Widget *create_chart_webview(Gtk::Window &window);
#endif // CHART_WEBVIEW_H
