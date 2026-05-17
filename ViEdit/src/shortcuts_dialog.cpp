#include "shortcuts_dialog.hpp"

namespace ViEdit {

ShortcutsDialog::ShortcutsDialog(Gtk::Window& parent) : Gtk::Dialog() {
    set_transient_for(parent);
    set_modal(true);
    set_title("Keyboard Shortcuts");
    set_default_size(450, 500);

    auto* btn = add_button("Close", Gtk::ResponseType::CLOSE);
    btn->signal_clicked().connect([this]() { close(); });

    setup_ui();
}

ShortcutsDialog::~ShortcutsDialog() = default;

void ShortcutsDialog::setup_ui() {
    m_content_area = get_content_area();

    m_scrolled_window = Gtk::make_managed<Gtk::ScrolledWindow>();
    m_scrolled_window->set_expand(true);
    m_scrolled_window->set_margin_top(10);
    m_scrolled_window->set_margin_bottom(10);
    m_scrolled_window->set_margin_start(10);
    m_scrolled_window->set_margin_end(10);

    m_grid = Gtk::make_managed<Gtk::Grid>();
    m_grid->set_column_spacing(20);
    m_grid->set_row_spacing(8);
    m_grid->set_margin_top(10);
    m_grid->set_margin_bottom(10);
    m_grid->set_margin_start(10);
    m_grid->set_margin_end(10);

    m_scrolled_window->set_child(*m_grid);
    m_content_area->append(*m_scrolled_window);

    auto title = Gtk::make_managed<Gtk::Label>("<b>File</b>");
    title->set_use_markup(true);
    title->set_halign(Gtk::Align::START);
    m_grid->attach(*title, 0, 0, 2, 1);

    int row = 1;
    add_shortcut(m_grid, row++, "Ctrl + N", "New File");
    add_shortcut(m_grid, row++, "Ctrl + O", "Open File");
    add_shortcut(m_grid, row++, "Ctrl + S", "Save");
    add_shortcut(m_grid, row++, "Ctrl + Shift + S", "Save As");
    add_shortcut(m_grid, row++, "Ctrl + W", "Close Tab");
    add_shortcut(m_grid, row++, "Ctrl + Q", "Quit");

    row++;
    auto title_edit = Gtk::make_managed<Gtk::Label>("<b>Edit</b>");
    title_edit->set_use_markup(true);
    title_edit->set_halign(Gtk::Align::START);
    m_grid->attach(*title_edit, 0, row++, 2, 1);

    add_shortcut(m_grid, row++, "Ctrl + Z", "Undo");
    add_shortcut(m_grid, row++, "Ctrl + Y", "Redo");
    add_shortcut(m_grid, row++, "Ctrl + F", "Find");
    add_shortcut(m_grid, row++, "Ctrl + H", "Replace");
    add_shortcut(m_grid, row++, "Ctrl + G", "Go to Line");

    row++;
    auto title_view = Gtk::make_managed<Gtk::Label>("<b>View</b>");
    title_view->set_use_markup(true);
    title_view->set_halign(Gtk::Align::START);
    m_grid->attach(*title_view, 0, row++, 2, 1);

    add_shortcut(m_grid, row++, "Ctrl + Scroll", "Zoom In/Out");
    add_shortcut(m_grid, row++, "Alt + Left", "Previous Tab");
    add_shortcut(m_grid, row++, "Alt + Right", "Next Tab");
}

void ShortcutsDialog::add_shortcut(Gtk::Grid* grid, int row, const Glib::ustring& shortcut, const Glib::ustring& description) {
    auto shortcut_label = Gtk::make_managed<Gtk::Label>(shortcut);
    shortcut_label->set_halign(Gtk::Align::END);
    shortcut_label->set_selectable(true);

    auto desc_label = Gtk::make_managed<Gtk::Label>(description);
    desc_label->set_halign(Gtk::Align::START);

    grid->attach(*shortcut_label, 0, row, 1, 1);
    grid->attach(*desc_label, 1, row, 1, 1);
}

} // namespace ViEdit
