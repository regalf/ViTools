#include "config.hpp"
#include <filesystem>
#include <fstream>
#include <functional>
#include <algorithm>
#include <glib.h>

namespace fs = std::filesystem;

namespace ViEdit {

Config& Config::instance() {
    static Config instance;
    return instance;
}

Config::Config() {
    m_settings = Gio::Settings::create("com.vitools.viedit");
}

Config::~Config() = default;

Glib::RefPtr<Gio::Settings> Config::get_settings() {
    return m_settings;
}

int Config::get_window_width() { return m_settings->get_int("window-width"); }
void Config::set_window_width(int width) { m_settings->set_int("window-width", width); }

int Config::get_window_height() { return m_settings->get_int("window-height"); }
void Config::set_window_height(int height) { m_settings->set_int("window-height", height); }

bool Config::get_window_maximized() { return m_settings->get_boolean("window-maximized"); }
void Config::set_window_maximized(bool maximized) { m_settings->set_boolean("window-maximized", maximized); }

bool Config::get_dark_theme() { return m_settings->get_boolean("dark-theme"); }
void Config::set_dark_theme(bool dark) { m_settings->set_boolean("dark-theme", dark); }

int Config::get_font_size() { return m_settings->get_int("font-size"); }
void Config::set_font_size(int size) { m_settings->set_int("font-size", size); }

Glib::ustring Config::get_font_family() { return m_settings->get_string("font-family"); }
void Config::set_font_family(const Glib::ustring& family) { m_settings->set_string("font-family", family.c_str()); }

bool Config::get_show_line_numbers() { return m_settings->get_boolean("show-line-numbers"); }
void Config::set_show_line_numbers(bool show) { m_settings->set_boolean("show-line-numbers", show); }

bool Config::get_word_wrap() { return m_settings->get_boolean("word-wrap"); }
void Config::set_word_wrap(bool wrap) { m_settings->set_boolean("word-wrap", wrap); }

bool Config::get_highlight_syntax() { return m_settings->get_boolean("highlight-syntax"); }
void Config::set_highlight_syntax(bool highlight) { m_settings->set_boolean("highlight-syntax", highlight); }

bool Config::get_auto_indent() { return m_settings->get_boolean("auto-indent"); }
void Config::set_auto_indent(bool indent) { m_settings->set_boolean("auto-indent", indent); }

std::vector<std::string> Config::get_recent_files() {
    std::vector<std::string> result;
    Glib::VariantBase base;
    m_settings->get_value("recent-files", base);
    auto variant = Glib::VariantBase::cast_dynamic<Glib::Variant<std::vector<Glib::ustring>>>(base);
    auto vec = variant.get();
    for (auto& s : vec) {
        result.push_back(s);
    }
    return result;
}

void Config::add_recent_file(const std::string& path) {
    auto recent = get_recent_files();
    recent.erase(std::remove(recent.begin(), recent.end(), path), recent.end());
    recent.insert(recent.begin(), path);

    int max = get_max_recent_files();
    if (static_cast<int>(recent.size()) > max) {
        recent.resize(max);
    }

    std::vector<Glib::ustring> glist;
    for (auto& f : recent) {
        glist.push_back(f);
    }
    auto variant = Glib::Variant<std::vector<Glib::ustring>>::create(glist);
    m_settings->set_value("recent-files", variant);
}

void Config::remove_recent_file(const std::string& path) {
    auto recent = get_recent_files();
    recent.erase(std::remove(recent.begin(), recent.end(), path), recent.end());

    std::vector<Glib::ustring> glist;
    for (auto& f : recent) {
        glist.push_back(f);
    }
    auto variant = Glib::Variant<std::vector<Glib::ustring>>::create(glist);
    m_settings->set_value("recent-files", variant);
}

void Config::clear_recent_files() {
    auto variant = Glib::Variant<std::vector<Glib::ustring>>::create({});
    m_settings->set_value("recent-files", variant);
}

int Config::get_max_recent_files() { return m_settings->get_int("max-recent-files"); }
void Config::set_max_recent_files(int max) { m_settings->set_int("max-recent-files", max); }

int Config::get_tab_size() { return m_settings->get_int("tab-size"); }
void Config::set_tab_size(int size) { m_settings->set_int("tab-size", size); }

bool Config::get_use_spaces_for_tabs() { return m_settings->get_boolean("use-spaces-for-tabs"); }
void Config::set_use_spaces_for_tabs(bool spaces) { m_settings->set_boolean("use-spaces-for-tabs", spaces); }

std::string Config::get_draft_path() {
    const char* config_dir = g_get_user_config_dir();
    return std::string(config_dir) + "/vitools/viedit/drafts";
}

std::string Config::get_draft_file_for_path(const std::string& file_path) {
    std::string hash = std::to_string(std::hash<std::string>{}(file_path));
    return get_draft_path() + "/" + hash + ".draft";
}

void Config::save_draft(const std::string& file_path, const std::string& content) {
    std::string draft_path = get_draft_file_for_path(file_path);
    fs::create_directories(fs::path(draft_path).parent_path());
    std::ofstream out(draft_path);
    if (out.is_open()) {
        out << "#DRAFT:" << file_path << "\n";
        out << content;
        out.close();
    }
}

std::string Config::load_draft(const std::string& file_path) {
    std::string draft_path = get_draft_file_for_path(file_path);
    std::ifstream in(draft_path);
    if (in.is_open()) {
        std::string line;
        std::getline(in, line);
        if (line.rfind("#DRAFT:", 0) == 0) {
            return std::string((std::istreambuf_iterator<char>(in)),
                               std::istreambuf_iterator<char>());
        }
        in.seekg(0);
        return std::string((std::istreambuf_iterator<char>(in)),
                           std::istreambuf_iterator<char>());
    }
    return "";
}

bool Config::has_draft(const std::string& file_path) {
    std::string draft_path = get_draft_file_for_path(file_path);
    return fs::exists(draft_path);
}

void Config::delete_draft(const std::string& file_path) {
    std::string draft_path = get_draft_file_for_path(file_path);
    if (fs::exists(draft_path)) {
        fs::remove(draft_path);
    }
}

std::vector<std::string> Config::list_all_drafts() {
    std::vector<std::string> result;
    std::string draft_dir = get_draft_path();
    if (!fs::exists(draft_dir)) return result;

    for (const auto& entry : fs::directory_iterator(draft_dir)) {
        if (entry.path().extension() == ".draft") {
            std::ifstream in(entry.path());
            if (in.is_open()) {
                std::string line;
                std::getline(in, line);
                if (line.rfind("#DRAFT:", 0) == 0) {
                    result.push_back(line.substr(7));
                }
            }
        }
    }
    return result;
}

std::string Config::get_draft_path_for_file(const std::string& draft_file) {
    std::ifstream in(draft_file);
    if (in.is_open()) {
        std::string line;
        std::getline(in, line);
        if (line.rfind("#DRAFT:", 0) == 0) {
            return line.substr(7);
        }
    }
    return "";
}

} // namespace ViEdit
