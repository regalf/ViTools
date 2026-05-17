#ifndef VITERM_ABOUT_DIALOG_HPP
#define VITERM_ABOUT_DIALOG_HPP

#include <gtkmm/window.h>
#include <gtkmm/label.h>
#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/linkbutton.h>

namespace ViTerm {

class AboutDialog : public Gtk::Window {
public:
    explicit AboutDialog(Gtk::Window& parent);
    ~AboutDialog() override;

private:
    void setup_ui();
};

} // namespace ViTerm

#endif // VITERM_ABOUT_DIALOG_HPP
