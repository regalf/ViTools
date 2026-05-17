#ifndef VIEDIT_PREFERENCES_DIALOG_HPP
#define VIEDIT_PREFERENCES_DIALOG_HPP

#include <gtkmm/dialog.h>
#include <gtkmm/box.h>
#include <gtkmm/label.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/fontbutton.h>
#include <gtkmm/grid.h>

namespace ViEdit {

class PreferencesDialog : public Gtk::Dialog {
public:
    explicit PreferencesDialog(Gtk::Window& parent);
    ~PreferencesDialog() override;

    void save_settings();

private:
    void setup_ui();
    void load_settings();

    Gtk::Box* m_content_area = nullptr;
    Gtk::Grid* m_grid = nullptr;

    Gtk::CheckButton* m_dark_theme_check = nullptr;
    Gtk::FontButton* m_font_button = nullptr;
    Gtk::SpinButton* m_font_size_spin = nullptr;
    Gtk::CheckButton* m_line_numbers_check = nullptr;
    Gtk::CheckButton* m_word_wrap_check = nullptr;
    Gtk::CheckButton* m_highlight_check = nullptr;
    Gtk::CheckButton* m_auto_indent_check = nullptr;
    Gtk::SpinButton* m_tab_size_spin = nullptr;
    Gtk::CheckButton* m_spaces_tabs_check = nullptr;
};

} // namespace ViEdit

#endif // VIEDIT_PREFERENCES_DIALOG_HPP
