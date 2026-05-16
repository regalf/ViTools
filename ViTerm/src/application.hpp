#ifndef VITERM_APPLICATION_HPP
#define VITERM_APPLICATION_HPP

#include <gtkmm/application.h>

namespace ViTerm {

class Application : public Gtk::Application {
public:
    static Glib::RefPtr<Application> create();

protected:
    Application();
    void on_activate() override;
    void on_startup() override;

private:
    void setup_actions();
    void setup_accels();
};

} // namespace ViTerm

#endif // VITERM_APPLICATION_HPP
