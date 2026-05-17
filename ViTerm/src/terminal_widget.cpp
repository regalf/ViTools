#include "terminal_widget.hpp"
#include "config.hpp"
#include <giomm/file.h>
#include <gtkmm/eventcontrollermotion.h>
#include <iostream>

namespace ViTerm {

TerminalWidget::TerminalWidget() {
    setup_terminal();
    setup_drag_and_drop();
}

TerminalWidget::~TerminalWidget() {
    if (m_terminal) {
        g_object_unref(m_terminal);
    }
}

void TerminalWidget::setup_terminal() {
    m_terminal = VTE_TERMINAL(vte_terminal_new());

    const char* shell = g_getenv("SHELL");
    if (!shell) {
        shell = "/bin/bash";
    }

    char* argv[] = { g_strdup(shell), nullptr };
    vte_terminal_spawn_async(
        m_terminal,
        VTE_PTY_DEFAULT,
        nullptr,
        argv,
        nullptr,
        G_SPAWN_SEARCH_PATH,
        nullptr,
        nullptr,
        nullptr,
        -1,
        nullptr,
        nullptr,
        nullptr
    );
    g_free(argv[0]);

    g_signal_connect(m_terminal, "child-exited",
                     G_CALLBACK(+[](VteTerminal*, int, gpointer user_data) {
                         auto* self = static_cast<TerminalWidget*>(user_data);
                         self->on_child_exited(self->m_terminal, 0);
                     }),
                     this);

    g_signal_connect(m_terminal, "window-title-changed",
                     G_CALLBACK(+[](VteTerminal*, gpointer user_data) {
                         auto* self = static_cast<TerminalWidget*>(user_data);
                         self->on_window_title_changed();
                     }),
                     this);

    auto* widget = GTK_WIDGET(m_terminal);
    gtk_widget_set_hexpand(widget, TRUE);
    gtk_widget_set_vexpand(widget, TRUE);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(gobj()), widget);

    auto* scroll_controller = gtk_event_controller_scroll_new(GTK_EVENT_CONTROLLER_SCROLL_BOTH_AXES);
    gtk_event_controller_set_propagation_phase(scroll_controller, GTK_PHASE_CAPTURE);
    g_signal_connect(scroll_controller, "scroll",
                     G_CALLBACK(+[](GtkEventControllerScroll* controller, gdouble, gdouble dy, gpointer user_data) -> gboolean {
                         auto* self = static_cast<TerminalWidget*>(user_data);
                         GdkEvent* event = gtk_event_controller_get_current_event(GTK_EVENT_CONTROLLER(controller));
                         if (event) {
                             GdkModifierType state = gdk_event_get_modifier_state(event);
                             if (state & GDK_CONTROL_MASK) {
                                 if (dy < 0) {
                                     self->zoom_font(1);
                                 } else if (dy > 0) {
                                     self->zoom_font(-1);
                                 }
                                 return GDK_EVENT_STOP;
                             }
                         }
                         return GDK_EVENT_PROPAGATE;
                     }),
                     this);
    gtk_widget_add_controller(widget, scroll_controller);
}

void TerminalWidget::setup_drag_and_drop() {
    auto* widget = GTK_WIDGET(m_terminal);

    auto* drop_target = gtk_drop_target_new(G_TYPE_STRING, GDK_ACTION_COPY);
    g_signal_connect(drop_target, "drop",
                     G_CALLBACK(+[](GtkDropTarget*, const GValue* value, double, double, gpointer user_data) -> gboolean {
                         auto* terminal = static_cast<TerminalWidget*>(user_data);
                         if (G_VALUE_HOLDS(value, G_TYPE_STRING)) {
                             const char* uri = g_value_get_string(value);
                             if (uri && g_str_has_prefix(uri, "file://")) {
                                 char* filename = g_filename_from_uri(uri, nullptr, nullptr);
                                 if (filename) {
                                     char* escaped = g_shell_quote(filename);
                                     vte_terminal_feed_child(terminal->m_terminal, escaped, -1);
                                     g_free(escaped);
                                     g_free(filename);
                                     return TRUE;
                                 }
                             }
                         }
                         return FALSE;
                     }),
                     this);

    gtk_widget_add_controller(widget, GTK_EVENT_CONTROLLER(drop_target));
}

void TerminalWidget::set_font(const std::string& family, int size) {
    m_font_family = family;
    m_font_size = size;
    if (m_terminal) {
        PangoFontDescription* font_desc = pango_font_description_new();
        pango_font_description_set_family(font_desc, family.c_str());
        pango_font_description_set_size(font_desc, size * PANGO_SCALE);
        vte_terminal_set_font(m_terminal, font_desc);
        pango_font_description_free(font_desc);
    }
}

void TerminalWidget::zoom_font(int delta) {
    m_font_size = std::max(6, std::min(72, m_font_size + delta));
    if (m_terminal) {
        PangoFontDescription* font_desc = pango_font_description_new();
        pango_font_description_set_family(font_desc, m_font_family.c_str());
        pango_font_description_set_size(font_desc, m_font_size * PANGO_SCALE);
        vte_terminal_set_font(m_terminal, font_desc);
        pango_font_description_free(font_desc);
    }
    auto& cfg = Config::instance();
    cfg.font_size = m_font_size;
    cfg.save();
}

void TerminalWidget::set_background_color(const std::string& color) {
    m_bg_color = color;
    if (m_terminal) {
        GdkRGBA rgba;
        if (gdk_rgba_parse(&rgba, color.c_str())) {
            vte_terminal_set_color_background(m_terminal, &rgba);
        }
    }
}

void TerminalWidget::on_child_exited(VteTerminal*, int status) {
    std::cerr << "Terminal child exited with status: " << status << std::endl;
    stop_title_scroll();
}

void TerminalWidget::setup_tab_hover() {
    if (!m_tab_box) return;
    
    auto motion_controller = Gtk::EventControllerMotion::create();
    motion_controller->signal_enter().connect([this](double, double) {
        m_hovering = true;
        if (m_window_title.length() > 15) {
            m_scroll_pos = 0;
            m_scroll_forward = true;
            start_title_scroll();
        }
    });
    motion_controller->signal_leave().connect([this]() {
        m_hovering = false;
        stop_title_scroll();
        if (m_tab_label && m_window_title.length() > 15) {
            m_tab_label->set_text(m_window_title.substr(0, 15));
        }
    });
    m_tab_box->add_controller(motion_controller);
}

void TerminalWidget::on_window_title_changed() {
    const char* title = vte_terminal_get_window_title(m_terminal);
    m_window_title = title ? title : "Terminal";
    
    stop_title_scroll();
    
    if (m_tab_label) {
        if (m_window_title.length() > 15) {
            m_tab_label->set_text(m_window_title.substr(0, 15));
            if (m_hovering) {
                m_scroll_pos = 0;
                m_scroll_forward = true;
                start_title_scroll();
            }
        } else {
            m_tab_label->set_text(m_window_title);
        }
    }
}

void TerminalWidget::start_title_scroll() {
    if (m_scroll_timer == 0) {
        m_scroll_timer = g_timeout_add(300, [](gpointer user_data) -> gboolean {
            auto* self = static_cast<TerminalWidget*>(user_data);
            self->update_title_scroll();
            return G_SOURCE_CONTINUE;
        }, this);
    }
}

void TerminalWidget::stop_title_scroll() {
    if (m_scroll_timer != 0) {
        g_source_remove(m_scroll_timer);
        m_scroll_timer = 0;
    }
}

void TerminalWidget::update_title_scroll() {
    if (!m_tab_label || m_window_title.length() <= 15) {
        stop_title_scroll();
        if (m_tab_label) m_tab_label->set_text(m_window_title);
        return;
    }

    int len = static_cast<int>(m_window_title.length());
    int max_pos = len - 15;
    
    if (m_scroll_forward) {
        m_scroll_pos++;
        if (m_scroll_pos >= max_pos) {
            m_scroll_forward = false;
        }
    } else {
        m_scroll_pos--;
        if (m_scroll_pos <= 0) {
            m_scroll_forward = true;
        }
    }
    
    m_tab_label->set_text(m_window_title.substr(m_scroll_pos, 15));
}

} // namespace ViTerm
