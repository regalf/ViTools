#ifndef VITERM_PREFERENCES_DIALOG_HPP
#define VITERM_PREFERENCES_DIALOG_HPP

#include <gtkmm/window.h>
#include <gtkmm/fontchooserwidget.h>
#include <gtkmm/colorbutton.h>
#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/label.h>
#include <gtkmm/scale.h>
#include <gtkmm/adjustment.h>
#include <string>

namespace ViTerm {

class PreferencesDialog : public Gtk::Window {
public:
    explicit PreferencesDialog(Gtk::Window& parent,
                               const std::string& current_font,
                               int current_size,
                               const std::string& current_bg_color,
                               double current_opacity);
    ~PreferencesDialog() override;

    std::string get_font_family() const;
    int get_font_size() const;
    std::string get_bg_color() const;
    double get_opacity() const;

private:
    void setup_ui();
    void on_response_ok();
    void on_response_cancel();

    Gtk::FontChooserWidget* m_font_chooser = nullptr;
    Gtk::ColorButton* m_color_button = nullptr;
    Gtk::Scale* m_opacity_scale = nullptr;

    std::string m_current_font;
    int m_current_size;
    std::string m_current_bg_color;
    double m_current_opacity;
};

} // namespace ViTerm

#endif // VITERM_PREFERENCES_DIALOG_HPP
