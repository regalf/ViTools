#ifndef VIEDIT_CONFIG_HPP
#define VIEDIT_CONFIG_HPP

#include <giomm/settings.h>
#include <glibmm/ustring.h>
#include <vector>
#include <string>

namespace ViEdit {

class Config {
public:
    static Config& instance();

    Glib::RefPtr<Gio::Settings> get_settings();

    int get_window_width();
    void set_window_width(int width);

    int get_window_height();
    void set_window_height(int height);

    bool get_window_maximized();
    void set_window_maximized(bool maximized);

    bool get_dark_theme();
    void set_dark_theme(bool dark);

    int get_font_size();
    void set_font_size(int size);

    Glib::ustring get_font_family();
    void set_font_family(const Glib::ustring& family);

    bool get_show_line_numbers();
    void set_show_line_numbers(bool show);

    bool get_word_wrap();
    void set_word_wrap(bool wrap);

    bool get_highlight_syntax();
    void set_highlight_syntax(bool highlight);

    bool get_auto_indent();
    void set_auto_indent(bool indent);

    std::vector<std::string> get_recent_files();
    void add_recent_file(const std::string& path);
    void remove_recent_file(const std::string& path);
    void clear_recent_files();

    int get_max_recent_files();
    void set_max_recent_files(int max);

    int get_tab_size();
    void set_tab_size(int size);

    bool get_use_spaces_for_tabs();
    void set_use_spaces_for_tabs(bool spaces);

    static std::string get_draft_path();
    static std::string get_draft_file_for_path(const std::string& file_path);
    static void save_draft(const std::string& file_path, const std::string& content);
    static std::string load_draft(const std::string& file_path);
    static bool has_draft(const std::string& file_path);
    static void delete_draft(const std::string& file_path);

    static std::vector<std::string> list_all_drafts();
    static std::string get_draft_path_for_file(const std::string& draft_file);

private:
    Config();
    ~Config();
    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;

    Glib::RefPtr<Gio::Settings> m_settings;
};

} // namespace ViEdit

#endif // VIEDIT_CONFIG_HPP
