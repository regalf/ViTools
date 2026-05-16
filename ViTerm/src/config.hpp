#ifndef VITERM_CONFIG_HPP
#define VITERM_CONFIG_HPP

#include <string>
#include <gio/gio.h>

namespace ViTerm {

class Config {
public:
    static Config& instance();

    void load();
    void save();

    bool dark_theme = true;
    double opacity = 1.0;
    std::string font_family = "Monospace";
    int font_size = 12;
    std::string bg_color = "#000000";

private:
    Config() = default;
    std::string get_config_path() const;
};

} // namespace ViTerm

#endif // VITERM_CONFIG_HPP
