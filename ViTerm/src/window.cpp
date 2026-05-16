#include "window.hpp"
#include "terminal_widget.hpp"
#include "preferences_dialog.hpp"
#include "shortcuts_dialog.hpp"
#include "config.hpp"
#include <gtkmm/label.h>
#include <gtkmm/box.h>
#include <gtkmm/settings.h>
#include <gtkmm/eventcontrollerkey.h>
#include <gtkmm/scale.h>
#include <gtkmm/adjustment.h>
#include <gtkmm/separator.h>
#include <iostream>

namespace ViTerm {

Window::Window(Gtk::Application& app) {
    auto app_ref = Glib::RefPtr<Gtk::Application>(&app, [](Gtk::Application*){});
    set_application(app_ref);
    set_default_size(900, 600);
    set_title("ViTerm");
    load_settings();
    setup_ui();
    setup_shortcuts();
    add_new_tab();
}

Window::~Window() = default;

void Window::setup_ui() {
    setup_headerbar();
    setup_notebook();

    auto* main_box = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL, 0);
    main_box->append(*m_notebook);
    set_child(*main_box);

    auto& cfg = Config::instance();
    if (cfg.dark_theme) {
        auto settings = Gtk::Settings::get_default();
        if (settings) {
            settings->property_gtk_application_prefer_dark_theme().set_value(true);
        }
    }
}

void Window::setup_headerbar() {
    m_headerbar = Gtk::make_managed<Gtk::HeaderBar>();

    m_new_tab_button = Gtk::make_managed<Gtk::Button>();
    m_new_tab_button->set_icon_name("list-add-symbolic");
    m_new_tab_button->signal_clicked().connect(sigc::mem_fun(*this, &Window::add_new_tab));
    m_headerbar->pack_start(*m_new_tab_button);

    m_menu_button = Gtk::make_managed<Gtk::Button>();
    m_menu_button->set_icon_name("open-menu-symbolic");
    m_menu_button->signal_clicked().connect([this]() {
        m_popover->popup();
    });

    m_menu_box = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL, 5);
    m_menu_box->set_margin_start(10);
    m_menu_box->set_margin_end(10);
    m_menu_box->set_margin_top(10);
    m_menu_box->set_margin_bottom(10);

    auto& cfg = Config::instance();
    auto* theme_btn = Gtk::make_managed<Gtk::Button>(cfg.dark_theme ? "Light Theme" : "Dark Theme");
    theme_btn->signal_clicked().connect([this, theme_btn]() {
        toggle_theme();
        auto& c = Config::instance();
        theme_btn->set_label(c.dark_theme ? "Light Theme" : "Dark Theme");
        m_popover->popdown();
    });

    auto* pref_btn = Gtk::make_managed<Gtk::Button>("Preferences");
    pref_btn->signal_clicked().connect([this]() {
        open_preferences();
        m_popover->popdown();
    });

    m_menu_box->append(*theme_btn);
    m_menu_box->append(*pref_btn);

    auto* sep1 = Gtk::make_managed<Gtk::Separator>(Gtk::Orientation::HORIZONTAL);
    m_menu_box->append(*sep1);

    auto* build_info_label = Gtk::make_managed<Gtk::Label>("ViTerm v0.1.0");
    build_info_label->set_halign(Gtk::Align::CENTER);
    build_info_label->set_margin_top(5);
    build_info_label->set_margin_bottom(5);
    m_menu_box->append(*build_info_label);

    auto* sep2 = Gtk::make_managed<Gtk::Separator>(Gtk::Orientation::HORIZONTAL);
    m_menu_box->append(*sep2);

    auto* shortcuts_btn = Gtk::make_managed<Gtk::Button>("Keyboard Shortcuts");
    shortcuts_btn->signal_clicked().connect([this]() {
        auto* dialog = new ShortcutsDialog(*this);
        dialog->signal_hide().connect([dialog]() { delete dialog; });
        dialog->show();
        m_popover->popdown();
    });
    m_menu_box->append(*shortcuts_btn);

    m_popover = Gtk::make_managed<Gtk::Popover>();
    m_popover->set_parent(*m_menu_button);
    m_popover->set_child(*m_menu_box);

    m_headerbar->pack_end(*m_menu_button);

    set_titlebar(*m_headerbar);
}

void Window::setup_notebook() {
    m_notebook = Gtk::make_managed<Gtk::Notebook>();
    m_notebook->set_scrollable(true);
    m_notebook->signal_switch_page().connect([this](Gtk::Widget*, guint) {
        auto page_num = m_notebook->get_current_page();
        auto* term = dynamic_cast<TerminalWidget*>(m_notebook->get_nth_page(page_num));
        if (term) {
            gtk_widget_grab_focus(GTK_WIDGET(term->get_terminal()));
        }
    });
}

void Window::setup_shortcuts() {
    auto event_controller = Gtk::EventControllerKey::create();
    gtk_event_controller_set_propagation_phase(GTK_EVENT_CONTROLLER(event_controller->gobj()), GTK_PHASE_CAPTURE);
    event_controller->signal_key_pressed().connect([this](guint keyval, guint, Gdk::ModifierType state) {
        bool alt = (state & Gdk::ModifierType::ALT_MASK) != Gdk::ModifierType(0);

        if (alt) {
            if (keyval == GDK_KEY_t) {
                add_new_tab();
                return true;
            } else if (keyval == GDK_KEY_w) {
                close_current_tab();
                return true;
            } else if (keyval == GDK_KEY_Left) {
                switch_tab_left();
                return true;
            } else if (keyval == GDK_KEY_Right) {
                switch_tab_right();
                return true;
            }
        }
        return false;
    }, false);
    add_controller(event_controller);
}

void Window::setup_actions() {
    m_action_group = Gio::SimpleActionGroup::create();

    auto action_new_tab = Gio::SimpleAction::create("new-tab");
    action_new_tab->signal_activate().connect([this](const Glib::VariantBase&) {
        add_new_tab();
    });
    m_action_group->add_action(action_new_tab);

    auto action_close_tab = Gio::SimpleAction::create("close-tab");
    action_close_tab->signal_activate().connect([this](const Glib::VariantBase&) {
        close_current_tab();
    });
    m_action_group->add_action(action_close_tab);

    insert_action_group("win", m_action_group);
}

void Window::add_new_tab() {
    auto& cfg = Config::instance();
    auto* terminal = new TerminalWidget();
    terminal->set_font(cfg.font_family, cfg.font_size);
    terminal->set_background_color(cfg.bg_color);

    auto* tab_label = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL, 5);
    auto page_num = m_notebook->get_n_pages() + 1;
    auto* label = Gtk::make_managed<Gtk::Label>("Terminal " + std::to_string(page_num));
    auto* close_btn = Gtk::make_managed<Gtk::Button>();
    close_btn->set_icon_name("window-close-symbolic");
    close_btn->set_has_frame(false);
    close_btn->signal_clicked().connect([this, terminal]() {
        int page = -1;
        for (int i = 0; i < m_notebook->get_n_pages(); ++i) {
            if (m_notebook->get_nth_page(i) == terminal) {
                page = i;
                break;
            }
        }
        if (page >= 0) {
            m_notebook->remove_page(page);
        }
    });

    tab_label->append(*label);
    tab_label->append(*close_btn);

    terminal->set_tab_label(tab_label, label);

    m_notebook->append_page(*terminal, *tab_label);
    m_notebook->set_current_page(m_notebook->get_n_pages() - 1);
}

void Window::close_current_tab() {
    auto page = m_notebook->get_current_page();
    if (page >= 0) {
        m_notebook->remove_page(page);
    }
}

void Window::switch_tab_left() {
    auto page = m_notebook->get_current_page();
    auto n_pages = m_notebook->get_n_pages();
    if (n_pages > 1) {
        auto new_page = (page > 0) ? page - 1 : n_pages - 1;
        m_notebook->set_current_page(new_page);
    }
}

void Window::switch_tab_right() {
    auto page = m_notebook->get_current_page();
    auto n_pages = m_notebook->get_n_pages();
    if (n_pages > 1) {
        auto new_page = (page < n_pages - 1) ? page + 1 : 0;
        m_notebook->set_current_page(new_page);
    }
}

void Window::toggle_theme() {
    auto& cfg = Config::instance();
    cfg.dark_theme = !cfg.dark_theme;
    auto settings = Gtk::Settings::get_default();
    if (settings) {
        settings->property_gtk_application_prefer_dark_theme().set_value(cfg.dark_theme);
    }
    save_settings();
}

void Window::open_preferences() {
    auto& cfg = Config::instance();
    auto* dialog = new PreferencesDialog(*this, cfg.font_family, cfg.font_size, cfg.bg_color, cfg.opacity);
    dialog->signal_hide().connect([this, dialog]() {
        auto& c = Config::instance();
        c.font_family = dialog->get_font_family();
        c.font_size = dialog->get_font_size();
        c.bg_color = dialog->get_bg_color();
        c.opacity = dialog->get_opacity();

        set_opacity(c.opacity);

        for (int i = 0; i < m_notebook->get_n_pages(); ++i) {
            auto* term = dynamic_cast<TerminalWidget*>(m_notebook->get_nth_page(i));
            if (term) {
                term->set_font(c.font_family, c.font_size);
                term->set_background_color(c.bg_color);
            }
        }
        save_settings();
        delete dialog;
    });
    dialog->show();
}

void Window::load_settings() {
    auto& cfg = Config::instance();
    cfg.load();
    set_opacity(cfg.opacity);
}

void Window::save_settings() {
    auto& cfg = Config::instance();
    cfg.save();
}

} // namespace ViTerm
