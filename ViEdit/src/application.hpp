#ifndef VIEDIT_APPLICATION_HPP
#define VIEDIT_APPLICATION_HPP

#include <gtkmm/application.h>

namespace ViEdit {

class Application : public Gtk::Application {
public:
    static Glib::RefPtr<Application> create();

protected:
    Application();
    ~Application() override;

    void on_startup() override;
    void on_activate() override;

private:
    void setup_actions();
};

} // namespace ViEdit

#endif // VIEDIT_APPLICATION_HPP
