#ifndef VIEDIT_WINDOW_HPP
#define VIEDIT_WINDOW_HPP

#include <gtkmm/applicationwindow.h>
#include <gtkmm/notebook.h>
#include <gtkmm/headerbar.h>
#include <gtkmm/button.h>
#include <gtkmm/popover.h>
#include <gtkmm/box.h>
#include <gtkmm/label.h>
#include <gtkmm/revealer.h>
#include <gtkmm/searchentry.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/statusbar.h>
#include <giomm/simpleactiongroup.h>
#include <giomm/settings.h>
#include "editor_tab.hpp"

namespace ViEdit {

class Window : public Gtk::ApplicationWindow {
public:
    static Glib::RefPtr<Window> create(Gtk::Application& app);

    explicit Window(Gtk::Application& app);
    ~Window() override;

    void open_file(const std::string& path);

private:
    void setup_ui();
    void setup_headerbar();
    void setup_notebook();
    void setup_statusbar();
    void setup_find_bar();
    void setup_actions();
    void setup_shortcuts();
    void setup_event_controllers();

    void load_settings();
    void save_settings();
    void apply_theme();

    void new_file();
    void restore_drafts();
    void add_tab(EditorTab* tab);
    void open_file_dialog();
    void save_current_file();
    void save_current_file_as();
    void close_current_tab();
    void toggle_theme();
    void open_preferences();
    void open_shortcuts_dialog();
    void open_about_dialog();
    void toggle_highlight();
    void toggle_word_wrap();

    void show_find_bar();
    void show_replace_bar();
    void hide_find_bar();
    void find_next();
    void find_previous();
    void replace_current();
    void replace_all();

    void show_go_to_line_dialog();
    void go_to_line();

    void update_statusbar();
    void update_tab_labels();

    bool on_close_request() override;
    bool on_key_pressed(guint keyval, guint keycode, Gdk::ModifierType state);
    bool on_scroll_event(double dx, double dy);

    EditorTab* get_current_tab();
    GtkNotebook* get_notebook_gobj() { return m_notebook->gobj(); }

    Gtk::HeaderBar* m_headerbar = nullptr;
    Gtk::Notebook* m_notebook = nullptr;
    Gtk::Box* m_main_box = nullptr;
    Gtk::Box* m_statusbar = nullptr;

    Gtk::Revealer* m_find_revealer = nullptr;
    Gtk::Box* m_find_box = nullptr;
    Gtk::SearchEntry* m_find_entry = nullptr;
    Gtk::CheckButton* m_replace_check = nullptr;
    Gtk::Entry* m_replace_entry = nullptr;
    Gtk::Button* m_find_prev_btn = nullptr;
    Gtk::Button* m_find_next_btn = nullptr;
    Gtk::Button* m_replace_btn = nullptr;
    Gtk::Button* m_replace_all_btn = nullptr;
    Gtk::Button* m_close_find_btn = nullptr;

    Gtk::Button* m_new_tab_button = nullptr;
    Gtk::Button* m_menu_button = nullptr;
    Gtk::Popover* m_popover = nullptr;
    Gtk::Box* m_menu_box = nullptr;

    Gtk::Label* m_status_label = nullptr;
    Gtk::Label* m_position_label = nullptr;
    Gtk::Label* m_language_label = nullptr;
    Gtk::Label* m_modified_label = nullptr;
    Gtk::Label* m_wrap_label = nullptr;
    Gtk::Label* m_highlight_label = nullptr;

    Glib::RefPtr<Gio::SimpleActionGroup> m_action_group;
    Glib::RefPtr<Gio::Settings> m_settings;

    bool m_dark_theme = true;
    bool m_find_visible = false;
    bool m_replace_visible = false;

    int m_go_to_line_result = 0;
    Gtk::Entry* m_go_to_line_entry = nullptr;

    bool m_closing = false;
    int m_pending_closes = 0;
};

} // namespace ViEdit

#endif // VIEDIT_WINDOW_HPP
