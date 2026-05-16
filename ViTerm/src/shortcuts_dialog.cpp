#include "shortcuts_dialog.hpp"

namespace ViTerm {

ShortcutsDialog::ShortcutsDialog(Gtk::Window& parent) {
    set_transient_for(parent);
    set_modal(true);
    set_default_size(300, 400);
    set_title("Keyboard Shortcuts");
    setup_ui();
}

ShortcutsDialog::~ShortcutsDialog() = default;

void ShortcutsDialog::setup_ui() {
    auto* main_box = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL, 10);
    main_box->set_margin_start(20);
    main_box->set_margin_end(20);
    main_box->set_margin_top(20);
    main_box->set_margin_bottom(20);

    auto* listbox = Gtk::make_managed<Gtk::ListBox>();
    listbox->set_selection_mode(Gtk::SelectionMode::NONE);

    add_shortcut_row(listbox, "New Tab", "Alt + T");
    add_shortcut_row(listbox, "Close Tab", "Alt + W");
    add_shortcut_row(listbox, "Switch Tab Left", "Alt + Left");
    add_shortcut_row(listbox, "Switch Tab Right", "Alt + Right");
    add_shortcut_row(listbox, "Zoom In", "Ctrl + Scroll Up");
    add_shortcut_row(listbox, "Zoom Out", "Ctrl + Scroll Down");
    add_shortcut_row(listbox, "Quit", "Ctrl + Q");

    main_box->append(*listbox);

    auto* close_btn = Gtk::make_managed<Gtk::Button>("Close");
    close_btn->signal_clicked().connect([this]() { hide(); });
    close_btn->set_halign(Gtk::Align::END);
    main_box->append(*close_btn);

    set_child(*main_box);
}

void ShortcutsDialog::add_shortcut_row(Gtk::ListBox* listbox, const std::string& action, const std::string& keys) {
    auto* row_box = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL, 10);
    row_box->set_margin_start(10);
    row_box->set_margin_end(10);
    row_box->set_margin_top(5);
    row_box->set_margin_bottom(5);

    auto* action_label = Gtk::make_managed<Gtk::Label>(action);
    action_label->set_halign(Gtk::Align::START);
    action_label->set_hexpand(true);
    row_box->append(*action_label);

    auto* keys_label = Gtk::make_managed<Gtk::Label>(keys);
    keys_label->set_halign(Gtk::Align::END);
    row_box->append(*keys_label);

    auto* row = Gtk::make_managed<Gtk::ListBoxRow>();
    row->set_child(*row_box);
    row->set_selectable(false);
    listbox->append(*row);
}

} // namespace ViTerm
