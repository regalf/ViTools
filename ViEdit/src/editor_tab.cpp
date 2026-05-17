#include "editor_tab.hpp"
#include "config.hpp"
#include <fstream>
#include <filesystem>
#include <algorithm>

namespace fs = std::filesystem;

namespace ViEdit {

EditorTab::EditorTab() : Gtk::Box(Gtk::Orientation::VERTICAL) {
    setup_ui();
    apply_settings();
}

EditorTab::~EditorTab() {
    if (m_text_changed_conn.connected()) m_text_changed_conn.disconnect();
    if (m_cursor_moved_conn.connected()) m_cursor_moved_conn.disconnect();
}

void EditorTab::setup_ui() {
    m_buffer = gtk_source_buffer_new(nullptr);
    m_view = GTK_SOURCE_VIEW(gtk_source_view_new_with_buffer(m_buffer));

    gtk_text_view_set_monospace(GTK_TEXT_VIEW(m_view), TRUE);
    gtk_source_view_set_auto_indent(m_view, TRUE);
    gtk_source_view_set_insert_spaces_instead_of_tabs(m_view, TRUE);
    gtk_source_view_set_tab_width(m_view, 4);
    gtk_source_view_set_show_line_numbers(m_view, TRUE);
    gtk_source_view_set_highlight_current_line(m_view, FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(m_view), GTK_WRAP_NONE);

    m_style_provider = gtk_css_provider_new();
    gtk_style_context_add_provider_for_display(
        gtk_widget_get_display(GTK_WIDGET(m_view)),
        GTK_STYLE_PROVIDER(m_style_provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    m_scrolled_window = Gtk::make_managed<Gtk::ScrolledWindow>();
    m_scrolled_window->set_expand(true);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(m_scrolled_window->gobj()), GTK_WIDGET(m_view));

    append(*m_scrolled_window);

    g_signal_connect(m_buffer, "changed", G_CALLBACK(+[](GtkTextBuffer*, gpointer data) {
        static_cast<EditorTab*>(data)->on_text_changed();
    }), this);

    g_signal_connect(m_buffer, "mark-set", G_CALLBACK(+[](GtkTextBuffer*, GtkTextIter*, GtkTextMark*, gpointer data) {
        static_cast<EditorTab*>(data)->on_cursor_moved();
    }), this);

    set_theme_colors(true);
}

void EditorTab::load_file(const std::string& path) {
    m_file_path = path;
    m_is_draft = false;

    std::ifstream file(path);
    if (file.is_open()) {
        std::string content((std::istreambuf_iterator<char>(file)),
                            std::istreambuf_iterator<char>());
        file.close();
        gtk_text_buffer_set_text(GTK_TEXT_BUFFER(m_buffer), content.c_str(), -1);
    }

    gtk_text_buffer_set_modified(GTK_TEXT_BUFFER(m_buffer), FALSE);
    detect_language();
    load_draft_if_exists();
}

void EditorTab::load_draft(const std::string& path, const std::string& content) {
    m_file_path = path;
    m_is_draft = true;

    gtk_text_buffer_set_text(GTK_TEXT_BUFFER(m_buffer), content.c_str(), -1);
    gtk_text_buffer_set_modified(GTK_TEXT_BUFFER(m_buffer), TRUE);
    detect_language();
}

void EditorTab::load_draft_if_exists() {
    if (m_is_draft && Config::has_draft(m_file_path)) {
        std::string draft = Config::load_draft(m_file_path);
        if (!draft.empty()) {
            gtk_text_buffer_set_text(GTK_TEXT_BUFFER(m_buffer), draft.c_str(), -1);
        }
    }
}

bool EditorTab::save_file() {
    if (m_file_path.empty()) {
        return false;
    }
    return save_file_as(m_file_path);
}

bool EditorTab::save_file_as(const std::string& path) {
    std::ofstream file(path);
    if (!file.is_open()) {
        return false;
    }

    GtkTextIter start, end;
    gtk_text_buffer_get_start_iter(GTK_TEXT_BUFFER(m_buffer), &start);
    gtk_text_buffer_get_end_iter(GTK_TEXT_BUFFER(m_buffer), &end);
    char* text = gtk_text_buffer_get_text(GTK_TEXT_BUFFER(m_buffer), &start, &end, FALSE);
    file << text;
    g_free(text);
    file.close();

    m_file_path = path;
    m_is_draft = false;
    gtk_text_buffer_set_modified(GTK_TEXT_BUFFER(m_buffer), FALSE);
    Config::delete_draft(path);
    Config::instance().add_recent_file(path);
    detect_language();

    return true;
}

void EditorTab::apply_settings() {
    auto& config = Config::instance();
    set_theme_colors(config.get_dark_theme());
    set_font_size(config.get_font_size());
    set_font_family(config.get_font_family());
    set_show_line_numbers(config.get_show_line_numbers());
    set_word_wrap(config.get_word_wrap());
    set_auto_indent(config.get_auto_indent());
    set_tab_size(config.get_tab_size());
    set_use_spaces_for_tabs(config.get_use_spaces_for_tabs());
    set_highlight_enabled(config.get_highlight_syntax());
}

bool EditorTab::is_modified() const {
    return gtk_text_buffer_get_modified(GTK_TEXT_BUFFER(m_buffer));
}

bool EditorTab::is_draft() const {
    return m_is_draft;
}

std::string EditorTab::get_file_path() const {
    return m_file_path;
}

Glib::ustring EditorTab::get_display_name() const {
    if (m_file_path.empty()) {
        return "Untitled";
    }
    std::string filename = fs::path(m_file_path).filename().string();
    if (m_is_draft || is_modified()) {
        return filename + " *";
    }
    return filename;
}

std::string EditorTab::get_language_name() const {
    GtkSourceLanguage* lang = gtk_source_buffer_get_language(m_buffer);
    if (lang) {
        return gtk_source_language_get_name(lang);
    }
    return "Plain Text";
}

int EditorTab::get_cursor_line() const {
    GtkTextIter iter;
    gtk_text_buffer_get_iter_at_mark(GTK_TEXT_BUFFER(m_buffer), &iter, gtk_text_buffer_get_insert(GTK_TEXT_BUFFER(m_buffer)));
    return gtk_text_iter_get_line(&iter) + 1;
}

int EditorTab::get_cursor_column() const {
    GtkTextIter iter;
    gtk_text_buffer_get_iter_at_mark(GTK_TEXT_BUFFER(m_buffer), &iter, gtk_text_buffer_get_insert(GTK_TEXT_BUFFER(m_buffer)));
    return gtk_text_iter_get_line_offset(&iter) + 1;
}

void EditorTab::set_highlight_enabled(bool enabled) {
    m_highlight_enabled = enabled;
    if (enabled) {
        detect_language();
    } else {
        gtk_source_buffer_set_language(m_buffer, nullptr);
    }
}

bool EditorTab::is_highlight_enabled() const {
    return m_highlight_enabled;
}

void EditorTab::set_font_size(int size) {
    set_theme_colors(m_is_dark);
}

void EditorTab::set_theme_colors(bool dark) {
    m_is_dark = dark;

    GtkSourceStyleSchemeManager* manager = gtk_source_style_scheme_manager_get_default();
    GtkSourceStyleScheme* scheme = nullptr;

    if (dark) {
        scheme = gtk_source_style_scheme_manager_get_scheme(manager, "Adwaita-dark");
        if (!scheme) scheme = gtk_source_style_scheme_manager_get_scheme(manager, "oblivion");
        if (!scheme) scheme = gtk_source_style_scheme_manager_get_scheme(manager, "solarized-dark");
    } else {
        scheme = gtk_source_style_scheme_manager_get_scheme(manager, "Adwaita");
        if (!scheme) scheme = gtk_source_style_scheme_manager_get_scheme(manager, "classic");
        if (!scheme) scheme = gtk_source_style_scheme_manager_get_scheme(manager, "solarized-light");
    }

    if (scheme) {
        gtk_source_buffer_set_style_scheme(m_buffer, scheme);
    }

    Glib::ustring font_family = Config::instance().get_font_family();
    int font_size = Config::instance().get_font_size();
    Glib::ustring css = Glib::ustring::compose(
        "textview {{ font: {0} {1}px; }}",
        font_family, font_size);
    gtk_css_provider_load_from_string(m_style_provider, css.c_str());
}

void EditorTab::set_font_family(const Glib::ustring& family) {
    set_theme_colors(m_is_dark);
}

void EditorTab::set_show_line_numbers(bool show) {
    gtk_source_view_set_show_line_numbers(m_view, show);
}

void EditorTab::set_word_wrap(bool wrap) {
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(m_view), wrap ? GTK_WRAP_WORD : GTK_WRAP_NONE);
}

void EditorTab::set_auto_indent(bool indent) {
    gtk_source_view_set_auto_indent(m_view, indent);
}

void EditorTab::set_tab_size(int size) {
    gtk_source_view_set_tab_width(m_view, size);
}

void EditorTab::set_use_spaces_for_tabs(bool spaces) {
    gtk_source_view_set_insert_spaces_instead_of_tabs(m_view, spaces);
}

void EditorTab::detect_language() {
    if (!m_highlight_enabled || m_file_path.empty()) {
        return;
    }

    GtkSourceLanguageManager* manager = gtk_source_language_manager_get_default();
    std::string ext = fs::path(m_file_path).extension().string();

    GtkSourceLanguage* lang = nullptr;

    if (ext == ".cpp" || ext == ".cxx" || ext == ".cc" || ext == ".hpp" || ext == ".hxx" || ext == ".hh") {
        lang = gtk_source_language_manager_get_language(manager, "cpp");
    } else if (ext == ".c" || ext == ".h") {
        lang = gtk_source_language_manager_get_language(manager, "c");
    } else if (ext == ".py") {
        lang = gtk_source_language_manager_get_language(manager, "python");
    } else if (ext == ".md") {
        lang = gtk_source_language_manager_get_language(manager, "markdown");
    } else if (ext == ".js") {
        lang = gtk_source_language_manager_get_language(manager, "javascript");
    } else if (ext == ".ts") {
        lang = gtk_source_language_manager_get_language(manager, "typescript");
    } else if (ext == ".html" || ext == ".htm") {
        lang = gtk_source_language_manager_get_language(manager, "html");
    } else if (ext == ".css") {
        lang = gtk_source_language_manager_get_language(manager, "css");
    } else if (ext == ".xml") {
        lang = gtk_source_language_manager_get_language(manager, "xml");
    } else if (ext == ".json") {
        lang = gtk_source_language_manager_get_language(manager, "json");
    } else if (ext == ".sh" || ext == ".bash") {
        lang = gtk_source_language_manager_get_language(manager, "sh");
    } else if (ext == ".rs") {
        lang = gtk_source_language_manager_get_language(manager, "rust");
    } else if (ext == ".java") {
        lang = gtk_source_language_manager_get_language(manager, "java");
    } else if (ext == ".rb") {
        lang = gtk_source_language_manager_get_language(manager, "ruby");
    } else if (ext == ".go") {
        lang = gtk_source_language_manager_get_language(manager, "go");
    } else if (ext == ".toml") {
        lang = gtk_source_language_manager_get_language(manager, "toml");
    } else if (ext == ".yaml" || ext == ".yml") {
        lang = gtk_source_language_manager_get_language(manager, "yaml");
    } else if (ext == ".cmake") {
        lang = gtk_source_language_manager_get_language(manager, "cmake");
    } else if (ext == ".meson") {
        lang = gtk_source_language_manager_get_language(manager, "meson");
    }

    if (lang) {
        gtk_source_buffer_set_language(m_buffer, lang);
    }
}

void EditorTab::find_text(const Glib::ustring& text, bool forward) {
    if (text.empty()) return;

    GtkTextIter iter;
    gtk_text_buffer_get_iter_at_mark(GTK_TEXT_BUFFER(m_buffer), &iter, gtk_text_buffer_get_insert(GTK_TEXT_BUFFER(m_buffer)));

    GtkTextIter match_start, match_end;

    if (forward) {
        gtk_text_iter_forward_chars(&iter, 1);
        if (gtk_text_iter_forward_search(&iter, text.c_str(), GTK_TEXT_SEARCH_CASE_INSENSITIVE, &match_start, &match_end, nullptr)) {
            gtk_text_buffer_select_range(GTK_TEXT_BUFFER(m_buffer), &match_start, &match_end);
            gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(m_view), &match_start, 0.0, FALSE, 0.0, 0.5);
        }
    } else {
        gtk_text_iter_backward_chars(&iter, 1);
        if (gtk_text_iter_backward_search(&iter, text.c_str(), GTK_TEXT_SEARCH_CASE_INSENSITIVE, &match_start, &match_end, nullptr)) {
            gtk_text_buffer_select_range(GTK_TEXT_BUFFER(m_buffer), &match_start, &match_end);
            gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(m_view), &match_start, 0.0, FALSE, 0.0, 0.5);
        }
    }
}

void EditorTab::replace_text(const Glib::ustring& search, const Glib::ustring& replace, bool replace_all) {
    if (search.empty()) return;

    if (replace_all) {
        GtkTextIter iter;
        gtk_text_buffer_get_start_iter(GTK_TEXT_BUFFER(m_buffer), &iter);
        GtkTextIter match_start, match_end;
        while (gtk_text_iter_forward_search(&iter, search.c_str(), GTK_TEXT_SEARCH_CASE_INSENSITIVE, &match_start, &match_end, nullptr)) {
            gtk_text_buffer_delete(GTK_TEXT_BUFFER(m_buffer), &match_start, &match_end);
            gtk_text_buffer_insert(GTK_TEXT_BUFFER(m_buffer), &match_start, replace.c_str(), -1);
            iter = match_end;
        }
    } else {
        GtkTextIter start, end;
        if (gtk_text_buffer_get_selection_bounds(GTK_TEXT_BUFFER(m_buffer), &start, &end)) {
            char* selected = gtk_text_buffer_get_text(GTK_TEXT_BUFFER(m_buffer), &start, &end, FALSE);
            if (selected && Glib::ustring(selected) == search) {
                gtk_text_buffer_delete(GTK_TEXT_BUFFER(m_buffer), &start, &end);
                gtk_text_buffer_insert(GTK_TEXT_BUFFER(m_buffer), &start, replace.c_str(), -1);
            }
            g_free(selected);
        }
        find_text(search, true);
    }
}

void EditorTab::go_to_line(int line) {
    if (line < 1) line = 1;
    GtkTextIter iter;
    gtk_text_buffer_get_iter_at_line(GTK_TEXT_BUFFER(m_buffer), &iter, line - 1);
    gtk_text_buffer_place_cursor(GTK_TEXT_BUFFER(m_buffer), &iter);
    gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(m_view), &iter, 0.0, FALSE, 0.0, 0.5);
}

sigc::signal<void()> EditorTab::signal_modified_changed() {
    return m_modified_changed_signal;
}

sigc::signal<void()> EditorTab::signal_cursor_moved() {
    return m_cursor_moved_signal;
}

void EditorTab::on_text_changed() {
    m_modified_changed_signal.emit();

    if (!m_file_path.empty() && is_modified()) {
        GtkTextIter start, end;
        gtk_text_buffer_get_start_iter(GTK_TEXT_BUFFER(m_buffer), &start);
        gtk_text_buffer_get_end_iter(GTK_TEXT_BUFFER(m_buffer), &end);
        char* text = gtk_text_buffer_get_text(GTK_TEXT_BUFFER(m_buffer), &start, &end, FALSE);
        Config::save_draft(m_file_path, text);
        g_free(text);
    }
}

void EditorTab::on_cursor_moved() {
    m_cursor_moved_signal.emit();
}

} // namespace ViEdit
