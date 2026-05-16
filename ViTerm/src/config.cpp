#include "config.hpp"
#include <glib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <cstdlib>

namespace ViTerm {

Config& Config::instance() {
    static Config config;
    return config;
}

std::string Config::get_config_path() const {
    const char* home = g_get_home_dir();
    return std::string(home) + "/.config/vitools/viterm.cfg";
}

void Config::load() {
    std::string path = get_config_path();
    GKeyFile* keyfile = g_key_file_new();

    if (g_key_file_load_from_file(keyfile, path.c_str(), G_KEY_FILE_NONE, nullptr)) {
        dark_theme = g_key_file_get_boolean(keyfile, "general", "dark_theme", nullptr);
        opacity = g_key_file_get_double(keyfile, "general", "opacity", nullptr);
        
        char* font = g_key_file_get_string(keyfile, "general", "font_family", nullptr);
        if (font) {
            font_family = font;
            g_free(font);
        }
        
        font_size = g_key_file_get_integer(keyfile, "general", "font_size", nullptr);
        
        char* color = g_key_file_get_string(keyfile, "general", "bg_color", nullptr);
        if (color) {
            bg_color = color;
            g_free(color);
        }
    }

    g_key_file_free(keyfile);
}

void Config::save() {
    std::string path = get_config_path();
    
    const char* home = g_get_home_dir();
    std::string config_dir = std::string(home) + "/.config/vitools";
    g_mkdir_with_parents(config_dir.c_str(), 0755);

    GKeyFile* keyfile = g_key_file_new();

    g_key_file_set_boolean(keyfile, "general", "dark_theme", dark_theme);
    g_key_file_set_double(keyfile, "general", "opacity", opacity);
    g_key_file_set_string(keyfile, "general", "font_family", font_family.c_str());
    g_key_file_set_integer(keyfile, "general", "font_size", font_size);
    g_key_file_set_string(keyfile, "general", "bg_color", bg_color.c_str());

    gsize length;
    char* data = g_key_file_to_data(keyfile, &length, nullptr);
    
    g_file_set_contents(path.c_str(), data, length, nullptr);
    
    g_free(data);
    g_key_file_free(keyfile);
}

} // namespace ViTerm
