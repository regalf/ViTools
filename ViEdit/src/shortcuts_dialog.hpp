#ifndef VIEDIT_SHORTCUTS_DIALOG_HPP
#define VIEDIT_SHORTCUTS_DIALOG_HPP

#include <gtkmm/dialog.h>
#include <gtkmm/box.h>
#include <gtkmm/label.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/grid.h>

namespace ViEdit {

class ShortcutsDialog : public Gtk::Dialog {
public:
    explicit ShortcutsDialog(Gtk::Window& parent);
    ~ShortcutsDialog() override;

private:
    void setup_ui();
    void add_shortcut(Gtk::Grid* grid, int row, const Glib::ustring& shortcut, const Glib::ustring& description);

    Gtk::Box* m_content_area = nullptr;
    Gtk::ScrolledWindow* m_scrolled_window = nullptr;
    Gtk::Grid* m_grid = nullptr;
};

} // namespace ViEdit

#endif // VIEDIT_SHORTCUTS_DIALOG_HPP
