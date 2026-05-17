#include "window.hpp"
#include "config.hpp"
#include "shortcuts_dialog.hpp"
#include "preferences_dialog.hpp"
#include <gtkmm/messagedialog.h>
#include <gtkmm/separator.h>
#include <gtkmm/eventcontrollerkey.h>
#include <gtkmm/eventcontrollerscroll.h>
#include <gdkmm/seat.h>
#include <gdk/gdkkeysyms.h>
#include <filesystem>
#include <functional>
#include <iostream>

namespace fs = std::filesystem;

namespace ViEdit {

Glib::RefPtr<Window> Window::create(Gtk::Application& app) {
    return Glib::make_refptr_for_instance<Window>(new Window(app));
}

Window::Window(Gtk::Application& app) {
    auto app_ptr = Glib::wrap(app.gobj(), true);
    set_application(app_ptr);

    std::cout << "[ViEdit] Starting..." << std::endl;

    setup_ui();
    std::cout << "[  OK  ] UI initialized" << std::endl;

    setup_actions();
    std::cout << "[  OK  ] Actions registered" << std::endl;

    setup_shortcuts();
    std::cout << "[  OK  ] Shortcuts configured" << std::endl;

    setup_event_controllers();
    std::cout << "[  OK  ] Event controllers ready" << std::endl;

    load_settings();
    std::cout << "[  OK  ] Settings loaded" << std::endl;

    apply_theme();
    std::cout << "[  OK  ] Theme applied" << std::endl;

    restore_drafts();
    std::cout << "[  OK  ] Session restored" << std::endl;

    std::cout << "[ViEdit] Ready" << std::endl;
}

Window::~Window() = default;

void Window::setup_ui() {
    set_default_size(Config::instance().get_window_width(), Config::instance().get_window_height());
    if (Config::instance().get_window_maximized()) {
        maximize();
    }

    m_main_box = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL);

    setup_headerbar();
    setup_notebook();
    setup_find_bar();
    setup_statusbar();

    m_main_box->append(*m_headerbar);
    m_main_box->append(*m_notebook);
    m_main_box->append(*m_find_revealer);
    m_main_box->append(*m_statusbar);

    set_child(*m_main_box);
}

void Window::setup_headerbar() {
    m_headerbar = Gtk::make_managed<Gtk::HeaderBar>();

    m_new_tab_button = Gtk::make_managed<Gtk::Button>();
    m_new_tab_button->set_label("New");
    m_new_tab_button->signal_clicked().connect(sigc::mem_fun(*this, &Window::new_file));
    m_headerbar->pack_start(*m_new_tab_button);

    m_menu_button = Gtk::make_managed<Gtk::Button>();
    m_menu_button->set_label("Menu");

    m_popover = Gtk::make_managed<Gtk::Popover>();
    m_popover->set_parent(*m_menu_button);
    m_menu_box = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL);
    m_menu_box->set_spacing(5);
    m_menu_box->set_margin_top(5);
    m_menu_box->set_margin_bottom(5);
    m_menu_box->set_margin_start(5);
    m_menu_box->set_margin_end(5);

    auto add_menu_item = [this](const Glib::ustring& label, sigc::slot<void()> callback) {
        auto btn = Gtk::make_managed<Gtk::Button>(label);
        btn->set_hexpand(true);
        btn->set_halign(Gtk::Align::FILL);
        btn->signal_clicked().connect(callback);
        btn->signal_clicked().connect([this]() { m_popover->popdown(); });
        m_menu_box->append(*btn);
    };

    add_menu_item("Open", sigc::mem_fun(*this, &Window::open_file_dialog));
    add_menu_item("Save", sigc::mem_fun(*this, &Window::save_current_file));
    add_menu_item("Save As", sigc::mem_fun(*this, &Window::save_current_file_as));
    m_menu_box->append(*Gtk::make_managed<Gtk::Separator>(Gtk::Orientation::HORIZONTAL));
    add_menu_item("Find", sigc::mem_fun(*this, &Window::show_find_bar));
    add_menu_item("Replace", sigc::mem_fun(*this, &Window::show_replace_bar));
    add_menu_item("Go to Line", sigc::mem_fun(*this, &Window::show_go_to_line_dialog));
    m_menu_box->append(*Gtk::make_managed<Gtk::Separator>(Gtk::Orientation::HORIZONTAL));
    add_menu_item("Preferences", sigc::mem_fun(*this, &Window::open_preferences));
    add_menu_item("Shortcuts", sigc::mem_fun(*this, &Window::open_shortcuts_dialog));
    m_menu_box->append(*Gtk::make_managed<Gtk::Separator>(Gtk::Orientation::HORIZONTAL));
    add_menu_item("About", sigc::mem_fun(*this, &Window::open_about_dialog));

    m_popover->set_child(*m_menu_box);
    m_menu_button->signal_clicked().connect([this]() { m_popover->popup(); });
    m_headerbar->pack_end(*m_menu_button);

    set_titlebar(*m_headerbar);
    set_title("ViEdit");
}

void Window::setup_notebook() {
    m_notebook = Gtk::make_managed<Gtk::Notebook>();
    m_notebook->set_scrollable(true);
    m_notebook->set_show_tabs(true);
    m_notebook->set_show_border(true);
}

void Window::setup_statusbar() {
    m_statusbar = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL);
    m_statusbar->set_margin_start(5);
    m_statusbar->set_margin_end(5);
    m_statusbar->set_margin_top(2);
    m_statusbar->set_margin_bottom(2);

    m_modified_label = Gtk::make_managed<Gtk::Label>("");
    m_modified_label->set_margin_end(10);

    m_language_label = Gtk::make_managed<Gtk::Label>("Plain Text");
    m_language_label->set_margin_end(10);

    m_position_label = Gtk::make_managed<Gtk::Label>("Ln 1, Col 1");
    m_position_label->set_margin_end(10);

    m_wrap_label = Gtk::make_managed<Gtk::Label>("");
    m_wrap_label->set_margin_end(10);

    m_highlight_label = Gtk::make_managed<Gtk::Label>("");
    m_highlight_label->set_margin_end(10);

    m_status_label = Gtk::make_managed<Gtk::Label>("Ready");

    m_statusbar->append(*m_modified_label);
    m_statusbar->append(*m_language_label);
    m_statusbar->append(*m_position_label);
    m_statusbar->append(*m_wrap_label);
    m_statusbar->append(*m_highlight_label);
    m_statusbar->append(*m_status_label);
}

void Window::setup_find_bar() {
    m_find_revealer = Gtk::make_managed<Gtk::Revealer>();
    m_find_revealer->set_reveal_child(false);
    m_find_revealer->set_transition_type(Gtk::RevealerTransitionType::SLIDE_DOWN);

    m_find_box = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL);
    m_find_box->set_spacing(5);
    m_find_box->set_margin_top(5);
    m_find_box->set_margin_bottom(5);
    m_find_box->set_margin_start(5);
    m_find_box->set_margin_end(5);

    m_find_entry = Gtk::make_managed<Gtk::SearchEntry>();
    m_find_entry->set_placeholder_text("Find");
    m_find_entry->set_hexpand(true);

    m_replace_check = Gtk::make_managed<Gtk::CheckButton>("Replace");
    m_replace_check->signal_toggled().connect([this]() {
        m_replace_visible = m_replace_check->get_active();
        m_replace_entry->set_visible(m_replace_visible);
        m_replace_btn->set_visible(m_replace_visible);
        m_replace_all_btn->set_visible(m_replace_visible);
    });

    m_replace_entry = Gtk::make_managed<Gtk::Entry>();
    m_replace_entry->set_placeholder_text("Replace");
    m_replace_entry->set_visible(false);

    m_find_prev_btn = Gtk::make_managed<Gtk::Button>("Previous");
    m_find_prev_btn->signal_clicked().connect(sigc::mem_fun(*this, &Window::find_previous));

    m_find_next_btn = Gtk::make_managed<Gtk::Button>("Next");
    m_find_next_btn->signal_clicked().connect(sigc::mem_fun(*this, &Window::find_next));

    m_replace_btn = Gtk::make_managed<Gtk::Button>("Replace");
    m_replace_btn->set_visible(false);
    m_replace_btn->signal_clicked().connect(sigc::mem_fun(*this, &Window::replace_current));

    m_replace_all_btn = Gtk::make_managed<Gtk::Button>("All");
    m_replace_all_btn->set_visible(false);
    m_replace_all_btn->signal_clicked().connect(sigc::mem_fun(*this, &Window::replace_all));

    m_close_find_btn = Gtk::make_managed<Gtk::Button>("X");
    m_close_find_btn->signal_clicked().connect(sigc::mem_fun(*this, &Window::hide_find_bar));

    m_find_box->append(*m_find_entry);
    m_find_box->append(*m_replace_entry);
    m_find_box->append(*m_replace_check);
    m_find_box->append(*m_find_prev_btn);
    m_find_box->append(*m_find_next_btn);
    m_find_box->append(*m_replace_btn);
    m_find_box->append(*m_replace_all_btn);
    m_find_box->append(*m_close_find_btn);

    m_find_revealer->set_child(*m_find_box);
}

void Window::setup_actions() {
    m_action_group = Gio::SimpleActionGroup::create();
    insert_action_group("win", Glib::RefPtr<Gio::ActionGroup>(m_action_group));

    auto add_action = [this](const Glib::ustring& name, std::function<void()> callback) {
        auto action = Gio::SimpleAction::create(name);
        action->signal_activate().connect([callback](const Glib::VariantBase&) { callback(); });
        m_action_group->add_action(action);
    };

    add_action("new_file", [this]() { new_file(); });
    add_action("open_file", [this]() { open_file_dialog(); });
    add_action("save", [this]() { save_current_file(); });
    add_action("save_as", [this]() { save_current_file_as(); });
    add_action("close_tab", [this]() { close_current_tab(); });
    add_action("quit", [this]() { on_close_request(); });
    add_action("find", [this]() { show_find_bar(); });
    add_action("replace", [this]() { show_replace_bar(); });
    add_action("go_to_line", [this]() { show_go_to_line_dialog(); });
    add_action("toggle_theme", [this]() { toggle_theme(); });
    add_action("toggle_highlight", [this]() { toggle_highlight(); });
    add_action("toggle_word_wrap", [this]() { toggle_word_wrap(); });
    add_action("preferences", [this]() { open_preferences(); });
    add_action("shortcuts", [this]() { open_shortcuts_dialog(); });
}

void Window::setup_shortcuts() {
    auto app = get_application();
    if (!app) return;

    app->set_accel_for_action("win.new_file", "<Primary>n");
    app->set_accel_for_action("win.open_file", "<Primary>o");
    app->set_accel_for_action("win.save", "<Primary>s");
    app->set_accel_for_action("win.save_as", "<Primary><Shift>s");
    app->set_accel_for_action("win.close_tab", "<Primary>w");
    app->set_accel_for_action("win.quit", "<Primary>q");
    app->set_accel_for_action("win.find", "<Primary>f");
    app->set_accel_for_action("win.replace", "<Primary>h");
    app->set_accel_for_action("win.go_to_line", "<Primary>g");
    app->set_accel_for_action("win.preferences", "<Primary>comma");
    app->set_accel_for_action("win.shortcuts", "<Primary>question");
}

void Window::setup_event_controllers() {
    auto key_controller = Gtk::EventControllerKey::create();
    key_controller->signal_key_pressed().connect_notify([this](guint keyval, guint keycode, Gdk::ModifierType state) {
        on_key_pressed(keyval, keycode, state);
    });
    add_controller(key_controller);

    auto scroll_controller = Gtk::EventControllerScroll::create();
    scroll_controller->set_flags(Gtk::EventControllerScroll::Flags::VERTICAL | Gtk::EventControllerScroll::Flags::DISCRETE);
    scroll_controller->signal_scroll().connect_notify([this](double dx, double dy) {
        on_scroll_event(dx, dy);
    });
    add_controller(scroll_controller);
}

void Window::load_settings() {
    auto& config = Config::instance();
    m_dark_theme = config.get_dark_theme();
}

void Window::save_settings() {
    auto& config = Config::instance();
    if (!is_maximized()) {
        int width, height;
        get_default_size(width, height);
        config.set_window_width(width);
        config.set_window_height(height);
    }
    config.set_window_maximized(is_maximized());
    config.set_dark_theme(m_dark_theme);
}

void Window::apply_theme() {
    auto* settings = gtk_settings_get_default();
    g_object_set(settings, "gtk-application-prefer-dark-theme", m_dark_theme ? TRUE : FALSE, NULL);
}

void Window::new_file() {
    auto* tab = Gtk::make_managed<EditorTab>();
    tab->apply_settings();
    add_tab(tab);
    m_notebook->set_current_page(m_notebook->get_n_pages() - 1);
    update_statusbar();
}

void Window::restore_drafts() {
    auto drafts = Config::list_all_drafts();
    auto recent = Config::instance().get_recent_files();

    if (drafts.empty() && recent.empty()) {
        new_file();
        std::cout << "[  OK  ] New file created" << std::endl;
        return;
    }

    int draft_count = 0;
    for (const auto& path : drafts) {
        std::string content = Config::load_draft(path);
        auto* tab = Gtk::make_managed<EditorTab>();
        tab->apply_settings();
        tab->load_draft(path, content);
        add_tab(tab);
        draft_count++;
    }

    int recent_count = 0;
    for (const auto& path : recent) {
        bool already_open = false;
        for (int i = 0; i < m_notebook->get_n_pages(); ++i) {
            auto* tab = dynamic_cast<EditorTab*>(m_notebook->get_nth_page(i));
            if (tab && tab->get_file_path() == path) {
                already_open = true;
                break;
            }
        }
        if (!already_open) {
            auto* tab = Gtk::make_managed<EditorTab>();
            tab->load_file(path);
            tab->apply_settings();
            add_tab(tab);
            recent_count++;
        }
    }

    m_notebook->set_current_page(0);
    update_statusbar();

    std::cout << "[  OK  ] Drafts restored: " << draft_count << std::endl;
    std::cout << "[  OK  ] Recent files: " << recent_count << std::endl;
    std::cout << "[  OK  ] Total tabs: " << m_notebook->get_n_pages() << std::endl;
}

void Window::add_tab(EditorTab* tab) {
    int page_num = m_notebook->append_page(*tab);
    m_notebook->set_tab_reorderable(*tab, true);

    auto tab_box = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL);
    tab_box->set_spacing(2);
    auto label = Gtk::make_managed<Gtk::Label>(tab->get_display_name());
    label->set_margin_start(5);
    auto close_btn = Gtk::make_managed<Gtk::Button>();
    close_btn->set_label("×");
    close_btn->set_focus_on_click(false);
    close_btn->set_size_request(18, 18);
    auto* close_provider = gtk_css_provider_new();
    gtk_css_provider_load_from_string(close_provider, "button.tab-close { border-radius: 9px; padding: 0; min-width: 18px; min-height: 18px; }");
    gtk_style_context_add_provider_for_display(
        gtk_widget_get_display(GTK_WIDGET(close_btn->gobj())),
        GTK_STYLE_PROVIDER(close_provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(close_provider);
    gtk_widget_add_css_class(GTK_WIDGET(close_btn->gobj()), "tab-close");
    close_btn->signal_clicked().connect([this, tab]() {
        for (int i = 0; i < m_notebook->get_n_pages(); ++i) {
            if (m_notebook->get_nth_page(i) == tab) {
                m_notebook->set_current_page(i);
                close_current_tab();
                break;
            }
        }
    });
    tab_box->append(*label);
    tab_box->append(*close_btn);
    m_notebook->set_tab_label(*tab, *tab_box);

    tab->signal_modified_changed().connect([this, tab, label]() {
        label->set_text(tab->get_display_name());
        update_statusbar();
    });

    tab->signal_cursor_moved().connect([this]() {
        update_statusbar();
    });
}

void Window::open_file_dialog() {
    auto* dialog = gtk_file_chooser_native_new("Open File", GTK_WINDOW(gobj()), GTK_FILE_CHOOSER_ACTION_OPEN, "_Open", "_Cancel");
    gtk_native_dialog_show(GTK_NATIVE_DIALOG(dialog));
    g_signal_connect(dialog, "response", G_CALLBACK(+[](GtkNativeDialog* dlg, int response, gpointer data) {
        auto* win = static_cast<Window*>(data);
        if (response == GTK_RESPONSE_ACCEPT) {
            auto* file = gtk_file_chooser_get_file(GTK_FILE_CHOOSER(dlg));
            if (file) {
                win->open_file(g_file_get_path(file));
                g_object_unref(file);
            }
        }
        gtk_native_dialog_destroy(dlg);
    }), this);
}

void Window::open_file(const std::string& path) {
    for (int i = 0; i < m_notebook->get_n_pages(); ++i) {
        auto* tab = dynamic_cast<EditorTab*>(m_notebook->get_nth_page(i));
        if (tab && tab->get_file_path() == path) {
            m_notebook->set_current_page(i);
            return;
        }
    }

    auto* tab = Gtk::make_managed<EditorTab>();
    tab->load_file(path);
    tab->apply_settings();
    add_tab(tab);
    m_notebook->set_current_page(m_notebook->get_n_pages() - 1);
    update_statusbar();
}

void Window::save_current_file() {
    auto* tab = get_current_tab();
    if (!tab) return;

    if (tab->is_draft() || tab->get_file_path().empty()) {
        save_current_file_as();
        return;
    }

    if (tab->save_file()) {
        update_tab_labels();
        update_statusbar();
    }
}

void Window::save_current_file_as() {
    auto* tab = get_current_tab();
    if (!tab) return;

    auto* dialog = gtk_file_chooser_native_new("Save File As", GTK_WINDOW(gobj()), GTK_FILE_CHOOSER_ACTION_SAVE, "_Save", "_Cancel");
    if (!tab->get_file_path().empty()) {
        gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), fs::path(tab->get_file_path()).filename().string().c_str());
    }
    gtk_native_dialog_show(GTK_NATIVE_DIALOG(dialog));
    g_signal_connect(dialog, "response", G_CALLBACK(+[](GtkNativeDialog* dlg, int response, gpointer data) {
        auto* win = static_cast<Window*>(data);
        if (response == GTK_RESPONSE_ACCEPT) {
            auto* file = gtk_file_chooser_get_file(GTK_FILE_CHOOSER(dlg));
            if (file) {
                auto* tab = win->get_current_tab();
                if (tab) {
                    std::string path = g_file_get_path(file);
                    if (tab->save_file_as(path)) {
                        win->update_tab_labels();
                        win->update_statusbar();
                    }
                }
                g_object_unref(file);
            }
        }
        gtk_native_dialog_destroy(dlg);
    }), this);
}

void Window::close_current_tab() {
    auto* tab = get_current_tab();
    if (!tab) return;

    if (tab->is_modified()) {
        auto* alert = gtk_alert_dialog_new("Unsaved Changes");
        gtk_alert_dialog_set_detail(alert, "Do you want to save before closing?");
        const char* buttons[] = {"_Save", "_Don't Save", "_Cancel", NULL};
        gtk_alert_dialog_set_buttons(alert, buttons);

        gtk_alert_dialog_choose(alert, GTK_WINDOW(gobj()), NULL, +[](GObject* source, GAsyncResult* res, gpointer data) {
            auto* win = static_cast<Window*>(data);
            int response = gtk_alert_dialog_choose_finish(GTK_ALERT_DIALOG(source), res, NULL);
            auto* nb = win->get_notebook_gobj();
            int page = gtk_notebook_get_current_page(nb);
            if (response == 0) {
                auto* t = win->get_current_tab();
                if (t && !t->get_file_path().empty()) {
                    t->save_file();
                }
                gtk_notebook_remove_page(nb, page);
                win->update_statusbar();
                if (gtk_notebook_get_n_pages(nb) == 0) {
                    win->save_settings();
                    std::cout << "[ViEdit] Exiting..." << std::endl;
                    gtk_window_close(GTK_WINDOW(win->gobj()));
                }
            } else if (response == 1) {
                auto* t = win->get_current_tab();
                std::string path = t ? t->get_file_path() : "";
                if (!path.empty()) {
                    Config::delete_draft(path);
                    Config::instance().remove_recent_file(path);
                }
                gtk_notebook_remove_page(nb, page);
                win->update_statusbar();
                if (gtk_notebook_get_n_pages(nb) == 0) {
                    win->save_settings();
                    std::cout << "[ViEdit] Exiting..." << std::endl;
                    gtk_window_close(GTK_WINDOW(win->gobj()));
                }
            }
        }, this);
        g_object_unref(alert);
        return;
    }

    int page = m_notebook->get_current_page();
    if (tab && !tab->get_file_path().empty()) {
        Config::delete_draft(tab->get_file_path());
        Config::instance().remove_recent_file(tab->get_file_path());
    }
    m_notebook->remove_page(page);
    update_statusbar();
    if (m_notebook->get_n_pages() == 0) {
        save_settings();
        std::cout << "[ViEdit] Exiting..." << std::endl;
        close();
    }
}

void Window::toggle_theme() {
    m_dark_theme = !m_dark_theme;
    Config::instance().set_dark_theme(m_dark_theme);
    apply_theme();
    for (int i = 0; i < m_notebook->get_n_pages(); ++i) {
        auto* tab = dynamic_cast<EditorTab*>(m_notebook->get_nth_page(i));
        if (tab) {
            tab->set_theme_colors(m_dark_theme);
        }
    }
}

void Window::open_preferences() {
    auto* dialog = Gtk::make_managed<PreferencesDialog>(*this);
    dialog->show();
    dialog->signal_response().connect([&, dialog](int response) {
        if (response == Gtk::ResponseType::OK) {
            dialog->save_settings();
            m_dark_theme = Config::instance().get_dark_theme();
            apply_theme();
            for (int i = 0; i < m_notebook->get_n_pages(); ++i) {
                auto* tab = dynamic_cast<EditorTab*>(m_notebook->get_nth_page(i));
                if (tab) {
                    tab->apply_settings();
                }
            }
        }
        dialog->close();
    });
}

void Window::open_shortcuts_dialog() {
    auto* dialog = Gtk::make_managed<ShortcutsDialog>(*this);
    dialog->show();
}

void Window::open_about_dialog() {
    auto dialog = Gtk::make_managed<Gtk::Window>();
    dialog->set_transient_for(*this);
    dialog->set_modal(true);
    dialog->set_title("About ViEdit");
    dialog->set_default_size(350, 250);

    auto box = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL);
    box->set_margin_top(20);
    box->set_margin_bottom(20);
    box->set_margin_start(20);
    box->set_margin_end(20);
    box->set_spacing(10);

    auto title = Gtk::make_managed<Gtk::Label>("<b>ViEdit</b>");
    title->set_use_markup(true);
    title->set_halign(Gtk::Align::CENTER);
    title->set_margin_top(10);

    auto version = Gtk::make_managed<Gtk::Label>("Version 0.1.0");
    version->set_halign(Gtk::Align::CENTER);

    auto desc = Gtk::make_managed<Gtk::Label>("A lightweight text editor for quick edits and note-taking.\nBuilt with GTK4 and C++20.");
    desc->set_halign(Gtk::Align::CENTER);
    desc->set_justify(Gtk::Justification::CENTER);
    desc->set_margin_top(10);

    auto link = Gtk::make_managed<Gtk::Label>("<a href=\"https://github.com/regalf/ViTools\">ViTools Project</a>");
    link->set_use_markup(true);
    link->set_halign(Gtk::Align::CENTER);
    link->set_margin_top(10);

    auto close_btn = Gtk::make_managed<Gtk::Button>("Close");
    close_btn->set_halign(Gtk::Align::CENTER);
    close_btn->set_margin_top(10);
    close_btn->signal_clicked().connect([dialog]() { dialog->close(); });

    box->append(*title);
    box->append(*version);
    box->append(*desc);
    box->append(*link);
    box->append(*close_btn);

    dialog->set_child(*box);
    dialog->show();
}

void Window::toggle_highlight() {
    auto* tab = get_current_tab();
    if (tab) {
        tab->set_highlight_enabled(!tab->is_highlight_enabled());
        Config::instance().set_highlight_syntax(tab->is_highlight_enabled());
        update_statusbar();
    }
}

void Window::toggle_word_wrap() {
    auto* tab = get_current_tab();
    if (tab) {
        auto& config = Config::instance();
        bool wrap = !config.get_word_wrap();
        config.set_word_wrap(wrap);
        tab->set_word_wrap(wrap);
    }
}

void Window::show_find_bar() {
    m_find_visible = true;
    m_replace_check->set_active(false);
    m_replace_visible = false;
    m_find_revealer->set_reveal_child(true);
    m_find_entry->grab_focus();
}

void Window::show_replace_bar() {
    m_find_visible = true;
    m_replace_check->set_active(true);
    m_replace_visible = true;
    m_find_revealer->set_reveal_child(true);
    m_find_entry->grab_focus();
}

void Window::hide_find_bar() {
    m_find_visible = false;
    m_find_revealer->set_reveal_child(false);
    m_find_entry->set_text("");
    m_replace_entry->set_text("");
}

void Window::find_next() {
    auto* tab = get_current_tab();
    if (tab) {
        tab->find_text(m_find_entry->get_text(), true);
    }
}

void Window::find_previous() {
    auto* tab = get_current_tab();
    if (tab) {
        tab->find_text(m_find_entry->get_text(), false);
    }
}

void Window::replace_current() {
    auto* tab = get_current_tab();
    if (tab) {
        tab->replace_text(m_find_entry->get_text(), m_replace_entry->get_text(), false);
    }
}

void Window::replace_all() {
    auto* tab = get_current_tab();
    if (tab) {
        tab->replace_text(m_find_entry->get_text(), m_replace_entry->get_text(), true);
    }
}

void Window::show_go_to_line_dialog() {
    auto dialog = Gtk::make_managed<Gtk::Window>();
    dialog->set_transient_for(*this);
    dialog->set_modal(true);
    dialog->set_title("Go to Line");
    dialog->set_default_size(250, 100);

    auto box = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL);
    box->set_margin_top(10);
    box->set_margin_bottom(10);
    box->set_margin_start(10);
    box->set_margin_end(10);
    box->set_spacing(10);

    auto label = Gtk::make_managed<Gtk::Label>("Line number:");
    auto entry = Gtk::make_managed<Gtk::Entry>();
    entry->set_text("1");

    auto btn_box = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL);
    btn_box->set_spacing(5);
    auto go_btn = Gtk::make_managed<Gtk::Button>("Go");
    auto cancel_btn = Gtk::make_managed<Gtk::Button>("Cancel");

    go_btn->signal_clicked().connect([&, dialog, entry]() {
        m_go_to_line_result = 1;
        try {
            int line = std::stoi(entry->get_text());
            auto* tab = get_current_tab();
            if (tab) {
                tab->go_to_line(line);
            }
        } catch (...) {
        }
        dialog->close();
    });
    cancel_btn->signal_clicked().connect([&, dialog]() {
        m_go_to_line_result = 0;
        dialog->close();
    });

    btn_box->append(*go_btn);
    btn_box->append(*cancel_btn);

    box->append(*label);
    box->append(*entry);
    box->append(*btn_box);

    dialog->set_child(*box);
    dialog->present();
}

void Window::go_to_line() {
    show_go_to_line_dialog();
}

void Window::update_statusbar() {
    auto* tab = get_current_tab();
    if (!tab) {
        m_status_label->set_text("No tabs open");
        m_position_label->set_text("");
        m_language_label->set_text("");
        m_modified_label->set_text("");
        return;
    }

    m_position_label->set_text(Glib::ustring::compose("Ln %1, Col %2", tab->get_cursor_line(), tab->get_cursor_column()));
    m_language_label->set_text(tab->get_language_name());
    m_modified_label->set_text(tab->is_modified() ? "Modified" : "");
    m_status_label->set_text(tab->get_file_path().empty() ? "New file" : tab->get_file_path());

    auto& config = Config::instance();
    m_wrap_label->set_text(config.get_word_wrap() ? "Wrap" : "");
    m_highlight_label->set_text(config.get_highlight_syntax() ? "Highlight" : "");
}

void Window::update_tab_labels() {
    for (int i = 0; i < m_notebook->get_n_pages(); ++i) {
        auto* tab = dynamic_cast<EditorTab*>(m_notebook->get_nth_page(i));
        auto* tab_box = dynamic_cast<Gtk::Box*>(m_notebook->get_tab_label(*tab));
        if (tab && tab_box) {
            auto* label = dynamic_cast<Gtk::Label*>(tab_box->get_first_child());
            if (label) {
                label->set_text(tab->get_display_name());
            }
        }
    }
}

bool Window::on_close_request() {
    if (m_closing) {
        return false;
    }

    for (int i = 0; i < m_notebook->get_n_pages(); ++i) {
        auto* tab = dynamic_cast<EditorTab*>(m_notebook->get_nth_page(i));
        if (tab && tab->is_modified()) {
            m_notebook->set_current_page(i);

            auto* alert = gtk_alert_dialog_new("Unsaved Changes");
            gtk_alert_dialog_set_detail(alert, "There are unsaved changes. Save before quitting?");
            const char* buttons[] = {"_Save", "_Don't Save", "_Cancel", NULL};
            gtk_alert_dialog_set_buttons(alert, buttons);

            gtk_alert_dialog_choose(alert, GTK_WINDOW(gobj()), NULL, +[](GObject* source, GAsyncResult* res, gpointer data) {
                auto* win = static_cast<Window*>(data);
                int response = gtk_alert_dialog_choose_finish(GTK_ALERT_DIALOG(source), res, NULL);
                if (response == 0) {
                    auto* tab = win->get_current_tab();
                    if (tab && !tab->get_file_path().empty()) {
                        tab->save_file();
                        win->m_closing = true;
                        win->save_settings();
                        gtk_window_close(GTK_WINDOW(win->gobj()));
                    } else if (tab) {
                        auto* dialog = gtk_file_chooser_native_new("Save File", GTK_WINDOW(win->gobj()), GTK_FILE_CHOOSER_ACTION_SAVE, "_Save", "_Cancel");
                        gtk_native_dialog_show(GTK_NATIVE_DIALOG(dialog));
                        g_signal_connect(dialog, "response", G_CALLBACK(+[](GtkNativeDialog* dlg, int resp, gpointer d) {
                            auto* w = static_cast<Window*>(d);
                            if (resp == GTK_RESPONSE_ACCEPT) {
                                auto* file = gtk_file_chooser_get_file(GTK_FILE_CHOOSER(dlg));
                                if (file) {
                                    auto* t = w->get_current_tab();
                                    if (t) t->save_file_as(g_file_get_path(file));
                                    g_object_unref(file);
                                }
                            }
                            w->m_closing = true;
                            w->save_settings();
                            gtk_window_close(GTK_WINDOW(w->gobj()));
                            gtk_native_dialog_destroy(dlg);
                        }), win);
                    }
                } else if (response == 1) {
                    win->m_closing = true;
                    win->save_settings();
                    gtk_window_close(GTK_WINDOW(win->gobj()));
                }
            }, this);
            g_object_unref(alert);
            return true;
        }
    }

    save_settings();
    std::cout << "[ViEdit] Exiting..." << std::endl;
    return false;
}

bool Window::on_key_pressed(guint keyval, guint, Gdk::ModifierType state) {
    auto state_int = static_cast<unsigned int>(state);
    auto meta_mask = static_cast<unsigned int>(Gdk::ModifierType::META_MASK);
    if (keyval == GDK_KEY_Left && (state_int & meta_mask)) {
        int page = m_notebook->get_current_page();
        if (page > 0) {
            m_notebook->set_current_page(page - 1);
        }
        return true;
    }
    if (keyval == GDK_KEY_Right && (state_int & meta_mask)) {
        int page = m_notebook->get_current_page();
        if (page < m_notebook->get_n_pages() - 1) {
            m_notebook->set_current_page(page + 1);
        }
        return true;
    }
    return false;
}

bool Window::on_scroll_event(double, double dy) {
    auto* tab = get_current_tab();
    if (!tab) return false;

    auto state = Gdk::ModifierType(0);
    auto display = Gdk::Display::get_default();
    if (display) {
        auto seat = display->get_default_seat();
        if (seat) {
            auto keyboard = seat->get_keyboard();
            if (keyboard) {
                state = keyboard->get_modifier_state();
            }
        }
    }
    auto state_int = static_cast<unsigned int>(state);
    if (state_int & static_cast<unsigned int>(Gdk::ModifierType::CONTROL_MASK)) {
        auto& config = Config::instance();
        int size = config.get_font_size();
        if (dy < 0) {
            size = std::min(size + 1, 72);
        } else {
            size = std::max(size - 1, 6);
        }
        config.set_font_size(size);
        tab->set_font_size(size);
        return true;
    }
    return false;
}

EditorTab* Window::get_current_tab() {
    int page = m_notebook->get_current_page();
    if (page < 0) return nullptr;
    return dynamic_cast<EditorTab*>(m_notebook->get_nth_page(page));
}

} // namespace ViEdit
