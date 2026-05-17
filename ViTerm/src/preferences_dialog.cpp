#include "preferences_dialog.hpp"
#include <gdkmm/rgba.h>
#include <iomanip>
#include <sstream>

namespace ViTerm {

PreferencesDialog::PreferencesDialog(Gtk::Window& parent,
                                     const std::string& current_font,
                                     int current_size,
                                     const std::string& current_bg_color,
                                     double current_opacity)
    : Gtk::Window(),
      m_current_font(current_font),
      m_current_size(current_size),
      m_current_bg_color(current_bg_color),
      m_current_opacity(current_opacity) {
    set_transient_for(parent);
    set_modal(true);
    set_default_size(400, 500);
    set_title("Preferences");
    setup_ui();
}

PreferencesDialog::~PreferencesDialog() = default;

void PreferencesDialog::setup_ui() {
    auto* main_box = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL, 15);
    main_box->set_margin_start(20);
    main_box->set_margin_end(20);
    main_box->set_margin_top(20);
    main_box->set_margin_bottom(20);

    auto* font_label = Gtk::make_managed<Gtk::Label>("Terminal Font");
    font_label->set_halign(Gtk::Align::START);
    main_box->append(*font_label);

    m_font_chooser = Gtk::make_managed<Gtk::FontChooserWidget>();
    
    Pango::FontDescription font_desc;
    font_desc.set_family(m_current_font);
    font_desc.set_size(m_current_size * PANGO_SCALE);
    m_font_chooser->set_font_desc(font_desc);
    
    main_box->append(*m_font_chooser);

    auto* color_label = Gtk::make_managed<Gtk::Label>("Background Color");
    color_label->set_halign(Gtk::Align::START);
    main_box->append(*color_label);

    m_color_button = Gtk::make_managed<Gtk::ColorButton>();
    Gdk::RGBA rgba;
    rgba.set(m_current_bg_color);
    m_color_button->set_rgba(rgba);
    m_color_button->set_halign(Gtk::Align::START);
    main_box->append(*m_color_button);

    auto* opacity_label = Gtk::make_managed<Gtk::Label>("Opacity");
    opacity_label->set_halign(Gtk::Align::START);
    main_box->append(*opacity_label);

    auto adj = Gtk::Adjustment::create(m_current_opacity, 0.5, 1.0, 0.01, 0.1, 0);
    m_opacity_scale = Gtk::make_managed<Gtk::Scale>(adj);
    m_opacity_scale->set_orientation(Gtk::Orientation::HORIZONTAL);
    m_opacity_scale->set_digits(2);
    main_box->append(*m_opacity_scale);

    auto* button_box = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL, 10);
    button_box->set_halign(Gtk::Align::END);
    button_box->set_margin_top(10);

    auto* cancel_btn = Gtk::make_managed<Gtk::Button>("Cancel");
    cancel_btn->signal_clicked().connect(sigc::mem_fun(*this, &PreferencesDialog::on_response_cancel));
    button_box->append(*cancel_btn);

    auto* ok_btn = Gtk::make_managed<Gtk::Button>("Apply");
    ok_btn->signal_clicked().connect(sigc::mem_fun(*this, &PreferencesDialog::on_response_ok));
    button_box->append(*ok_btn);

    main_box->append(*button_box);

    set_child(*main_box);
}

void PreferencesDialog::on_response_ok() {
    m_applied = true;
    hide();
}

void PreferencesDialog::on_response_cancel() {
    hide();
}

std::string PreferencesDialog::get_font_family() const {
    auto desc = m_font_chooser->get_font_desc();
    auto family = desc.get_family();
    return family.empty() ? "Monospace" : family;
}

int PreferencesDialog::get_font_size() const {
    auto desc = m_font_chooser->get_font_desc();
    return desc.get_size() / PANGO_SCALE;
}

std::string PreferencesDialog::get_bg_color() const {
    auto rgba = m_color_button->get_rgba();
    std::ostringstream oss;
    oss << "#"
        << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(rgba.get_red() * 255)
        << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(rgba.get_green() * 255)
        << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(rgba.get_blue() * 255);
    return oss.str();
}

double PreferencesDialog::get_opacity() const {
    return m_opacity_scale->get_value();
}

} // namespace ViTerm
