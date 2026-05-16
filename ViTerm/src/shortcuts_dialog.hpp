#ifndef VITERM_SHORTCUTS_DIALOG_HPP
#define VITERM_SHORTCUTS_DIALOG_HPP

#include <gtkmm/window.h>
#include <gtkmm/listbox.h>
#include <gtkmm/label.h>
#include <gtkmm/box.h>
#include <gtkmm/button.h>

namespace ViTerm {

class ShortcutsDialog : public Gtk::Window {
public:
    explicit ShortcutsDialog(Gtk::Window& parent);
    ~ShortcutsDialog() override;

private:
    void setup_ui();
    void add_shortcut_row(Gtk::ListBox* listbox, const std::string& action, const std::string& keys);
};

} // namespace ViTerm

#endif // VITERM_SHORTCUTS_DIALOG_HPP
