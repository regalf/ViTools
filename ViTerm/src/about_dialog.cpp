#include "about_dialog.hpp"

namespace ViTerm {

AboutDialog::AboutDialog(Gtk::Window& parent) {
    set_transient_for(parent);
    set_modal(true);
    set_default_size(300, 200);
    set_title("About ViTerm");
    setup_ui();
}

AboutDialog::~AboutDialog() = default;

void AboutDialog::setup_ui() {
    auto* main_box = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL, 10);
    main_box->set_margin_start(20);
    main_box->set_margin_end(20);
    main_box->set_margin_top(20);
    main_box->set_margin_bottom(20);

    auto* name_label = Gtk::make_managed<Gtk::Label>("ViTerm");
    name_label->set_halign(Gtk::Align::CENTER);
    name_label->set_hexpand(true);
    name_label->add_css_class("title-2");
    main_box->append(*name_label);

    auto* version_label = Gtk::make_managed<Gtk::Label>("v0.1.0");
    version_label->set_halign(Gtk::Align::CENTER);
    main_box->append(*version_label);

    auto* desc_label = Gtk::make_managed<Gtk::Label>("A modern terminal emulator built with GTK4 and C++20.");
    desc_label->set_halign(Gtk::Align::CENTER);
    desc_label->set_wrap(true);
    main_box->append(*desc_label);

    auto* link_button = Gtk::make_managed<Gtk::LinkButton>("https://github.com/regalf/ViTools", "ViTools Project");
    link_button->set_halign(Gtk::Align::CENTER);
    main_box->append(*link_button);

    auto* close_btn = Gtk::make_managed<Gtk::Button>("Close");
    close_btn->signal_clicked().connect([this]() { hide(); });
    close_btn->set_halign(Gtk::Align::CENTER);
    main_box->append(*close_btn);

    set_child(*main_box);
}

} // namespace ViTerm
