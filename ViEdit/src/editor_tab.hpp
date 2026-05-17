#ifndef VIEDIT_EDITOR_TAB_HPP
#define VIEDIT_EDITOR_TAB_HPP

#include <gtksourceview/gtksource.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/box.h>
#include <glibmm/ustring.h>
#include <string>

namespace ViEdit {

class EditorTab : public Gtk::Box {
public:
    explicit EditorTab();
    ~EditorTab() override;

    void load_file(const std::string& path);
    void load_draft(const std::string& path, const std::string& content);
    bool save_file();
    bool save_file_as(const std::string& path);
    void apply_settings();

    bool is_modified() const;
    bool is_draft() const;
    std::string get_file_path() const;
    Glib::ustring get_display_name() const;
    std::string get_language_name() const;
    int get_cursor_line() const;
    int get_cursor_column() const;

    void set_highlight_enabled(bool enabled);
    bool is_highlight_enabled() const;

    void set_font_size(int size);
    void set_font_family(const Glib::ustring& family);
    void set_theme_colors(bool dark);
    void set_show_line_numbers(bool show);
    void set_word_wrap(bool wrap);
    void set_auto_indent(bool indent);
    void set_tab_size(int size);
    void set_use_spaces_for_tabs(bool spaces);

    void find_text(const Glib::ustring& text, bool forward = true);
    void replace_text(const Glib::ustring& search, const Glib::ustring& replace, bool replace_all = false);
    void go_to_line(int line);

    sigc::signal<void()> signal_modified_changed();
    sigc::signal<void()> signal_cursor_moved();

private:
    void setup_ui();
    void detect_language();
    void load_draft_if_exists();
    void on_text_changed();
    void on_cursor_moved();

    Gtk::ScrolledWindow* m_scrolled_window = nullptr;
    GtkSourceBuffer* m_buffer = nullptr;
    GtkSourceView* m_view = nullptr;

    std::string m_file_path;
    bool m_is_draft = true;
    bool m_highlight_enabled = true;

    sigc::signal<void()> m_modified_changed_signal;
    sigc::signal<void()> m_cursor_moved_signal;

    sigc::connection m_text_changed_conn;
    sigc::connection m_cursor_moved_conn;

    GtkCssProvider* m_style_provider = nullptr;
    bool m_is_dark = true;
};

} // namespace ViEdit

#endif // VIEDIT_EDITOR_TAB_HPP
