#include "preferences_dialog.hpp"
#include "config.hpp"
#include <gtkmm/adjustment.h>

namespace ViEdit {

PreferencesDialog::PreferencesDialog(Gtk::Window& parent) : Gtk::Dialog() {
    set_transient_for(parent);
    set_modal(true);
    set_title("Preferences");
    set_default_size(400, 450);

    auto* cancel_btn = add_button("Cancel", Gtk::ResponseType::CANCEL);
    auto* save_btn = add_button("Save", Gtk::ResponseType::OK);
    cancel_btn->signal_clicked().connect([this]() { close(); });
    save_btn->signal_clicked().connect([this]() { close(); });

    setup_ui();
    load_settings();
}

PreferencesDialog::~PreferencesDialog() = default;

void PreferencesDialog::setup_ui() {
    m_content_area = get_content_area();

    m_grid = Gtk::make_managed<Gtk::Grid>();
    m_grid->set_column_spacing(10);
    m_grid->set_row_spacing(10);
    m_grid->set_margin_top(15);
    m_grid->set_margin_bottom(15);
    m_grid->set_margin_start(15);
    m_grid->set_margin_end(15);

    m_content_area->append(*m_grid);

    int row = 0;

    auto theme_label = Gtk::make_managed<Gtk::Label>("Dark Theme");
    theme_label->set_halign(Gtk::Align::START);
    m_dark_theme_check = Gtk::make_managed<Gtk::CheckButton>();
    m_grid->attach(*theme_label, 0, row, 1, 1);
    m_grid->attach(*m_dark_theme_check, 1, row++, 1, 1);

    auto font_label = Gtk::make_managed<Gtk::Label>("Font");
    font_label->set_halign(Gtk::Align::START);
    m_font_button = Gtk::make_managed<Gtk::FontButton>();
    m_grid->attach(*font_label, 0, row, 1, 1);
    m_grid->attach(*m_font_button, 1, row++, 1, 1);

    auto font_size_label = Gtk::make_managed<Gtk::Label>("Font Size");
    font_size_label->set_halign(Gtk::Align::START);
    m_font_size_spin = Gtk::make_managed<Gtk::SpinButton>(Gtk::Adjustment::create(8, 6, 72, 1));
    m_grid->attach(*font_size_label, 0, row, 1, 1);
    m_grid->attach(*m_font_size_spin, 1, row++, 1, 1);

    auto line_num_label = Gtk::make_managed<Gtk::Label>("Line Numbers");
    line_num_label->set_halign(Gtk::Align::START);
    m_line_numbers_check = Gtk::make_managed<Gtk::CheckButton>();
    m_grid->attach(*line_num_label, 0, row, 1, 1);
    m_grid->attach(*m_line_numbers_check, 1, row++, 1, 1);

    auto wrap_label = Gtk::make_managed<Gtk::Label>("Word Wrap");
    wrap_label->set_halign(Gtk::Align::START);
    m_word_wrap_check = Gtk::make_managed<Gtk::CheckButton>();
    m_grid->attach(*wrap_label, 0, row, 1, 1);
    m_grid->attach(*m_word_wrap_check, 1, row++, 1, 1);

    auto highlight_label = Gtk::make_managed<Gtk::Label>("Syntax Highlight");
    highlight_label->set_halign(Gtk::Align::START);
    m_highlight_check = Gtk::make_managed<Gtk::CheckButton>();
    m_grid->attach(*highlight_label, 0, row, 1, 1);
    m_grid->attach(*m_highlight_check, 1, row++, 1, 1);

    auto indent_label = Gtk::make_managed<Gtk::Label>("Auto Indent");
    indent_label->set_halign(Gtk::Align::START);
    m_auto_indent_check = Gtk::make_managed<Gtk::CheckButton>();
    m_grid->attach(*indent_label, 0, row, 1, 1);
    m_grid->attach(*m_auto_indent_check, 1, row++, 1, 1);

    auto tab_size_label = Gtk::make_managed<Gtk::Label>("Tab Size");
    tab_size_label->set_halign(Gtk::Align::START);
    m_tab_size_spin = Gtk::make_managed<Gtk::SpinButton>(Gtk::Adjustment::create(2, 1, 8, 1));
    m_grid->attach(*tab_size_label, 0, row, 1, 1);
    m_grid->attach(*m_tab_size_spin, 1, row++, 1, 1);

    auto spaces_label = Gtk::make_managed<Gtk::Label>("Spaces for Tabs");
    spaces_label->set_halign(Gtk::Align::START);
    m_spaces_tabs_check = Gtk::make_managed<Gtk::CheckButton>();
    m_grid->attach(*spaces_label, 0, row, 1, 1);
    m_grid->attach(*m_spaces_tabs_check, 1, row++, 1, 1);
}

void PreferencesDialog::load_settings() {
    auto& config = Config::instance();
    m_dark_theme_check->set_active(config.get_dark_theme());

    Glib::ustring font_desc = Glib::ustring::compose("%1 %2", config.get_font_family(), config.get_font_size());
    m_font_button->set_font(font_desc);
    m_font_size_spin->set_value(config.get_font_size());

    m_line_numbers_check->set_active(config.get_show_line_numbers());
    m_word_wrap_check->set_active(config.get_word_wrap());
    m_highlight_check->set_active(config.get_highlight_syntax());
    m_auto_indent_check->set_active(config.get_auto_indent());
    m_tab_size_spin->set_value(config.get_tab_size());
    m_spaces_tabs_check->set_active(config.get_use_spaces_for_tabs());
}

void PreferencesDialog::save_settings() {
    auto& config = Config::instance();
    config.set_dark_theme(m_dark_theme_check->get_active());

    auto font_desc = m_font_button->get_font();
    Pango::FontDescription fd(font_desc);
    config.set_font_family(fd.get_family());
    config.set_font_size(fd.get_size() / Pango::SCALE);
    m_font_size_spin->set_value(fd.get_size() / Pango::SCALE);

    config.set_show_line_numbers(m_line_numbers_check->get_active());
    config.set_word_wrap(m_word_wrap_check->get_active());
    config.set_highlight_syntax(m_highlight_check->get_active());
    config.set_auto_indent(m_auto_indent_check->get_active());
    config.set_tab_size(static_cast<int>(m_tab_size_spin->get_value()));
    config.set_use_spaces_for_tabs(m_spaces_tabs_check->get_active());
}

} // namespace ViEdit
