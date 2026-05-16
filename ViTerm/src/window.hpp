#ifndef VITERM_WINDOW_HPP
#define VITERM_WINDOW_HPP

#include <gtkmm/applicationwindow.h>
#include <gtkmm/notebook.h>
#include <gtkmm/headerbar.h>
#include <gtkmm/button.h>
#include <gtkmm/popover.h>
#include <gtkmm/box.h>
#include <giomm/simpleactiongroup.h>
#include <giomm/settings.h>

namespace ViTerm {

class Window : public Gtk::ApplicationWindow {
public:
    explicit Window(Gtk::Application& app);
    ~Window() override;

    void setup_actions();
    void add_new_tab();
    void close_current_tab();
    void switch_tab_left();
    void switch_tab_right();
    void toggle_theme();
    void open_preferences();

private:
    void setup_ui();
    void setup_headerbar();
    void setup_notebook();
    void setup_shortcuts();
    void load_settings();
    void save_settings();

    Gtk::HeaderBar* m_headerbar = nullptr;
    Gtk::Notebook* m_notebook = nullptr;
    Gtk::Button* m_new_tab_button = nullptr;
    Gtk::Button* m_menu_button = nullptr;
    Gtk::Popover* m_popover = nullptr;
    Gtk::Box* m_menu_box = nullptr;

    Glib::RefPtr<Gio::SimpleActionGroup> m_action_group;
};

} // namespace ViTerm

#endif // VITERM_WINDOW_HPP
