#ifndef VITERM_TERMINAL_WIDGET_HPP
#define VITERM_TERMINAL_WIDGET_HPP

#include <vte/vte.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/label.h>
#include <gtkmm/box.h>

namespace ViTerm {

class TerminalWidget : public Gtk::ScrolledWindow {
public:
    TerminalWidget();
    ~TerminalWidget() override;

    void set_font(const std::string& family, int size);
    void set_background_color(const std::string& color);
    void zoom_font(int delta);

    int get_font_size() const { return m_font_size; }
    const std::string& get_font_family() const { return m_font_family; }

    void set_tab_label(Gtk::Box* tab_box, Gtk::Label* label) {
        m_tab_label = label;
        m_tab_box = tab_box;
        setup_tab_hover();
    }
    VteTerminal* get_terminal() const { return m_terminal; }

private:
    void setup_terminal();
    void setup_drag_and_drop();
    void setup_tab_hover();
    void on_child_exited(VteTerminal*, int status);
    void start_title_scroll();
    void stop_title_scroll();
    void update_title_scroll();
    void on_window_title_changed();

    VteTerminal* m_terminal = nullptr;
    std::string m_bg_color = "#000000";
    std::string m_font_family = "Monospace";
    int m_font_size = 12;
    Gtk::Label* m_tab_label = nullptr;
    Gtk::Box* m_tab_box = nullptr;
    bool m_hovering = false;
    std::string m_window_title;
    guint m_scroll_timer = 0;
    int m_scroll_pos = 0;
    bool m_scroll_forward = true;
};

} // namespace ViTerm

#endif // VITERM_TERMINAL_WIDGET_HPP
