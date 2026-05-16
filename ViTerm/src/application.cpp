#include "application.hpp"
#include "window.hpp"

namespace ViTerm {

Glib::RefPtr<Application> Application::create() {
    return Glib::make_refptr_for_instance<Application>(
        new Application()
    );
}

Application::Application()
    : Gtk::Application("com.vitools.viterm") {
}

void Application::on_startup() {
    Gtk::Application::on_startup();
    setup_actions();
    setup_accels();
}

void Application::on_activate() {
    auto* window = dynamic_cast<Window*>(Gtk::Application::get_active_window());
    if (!window) {
        window = new ViTerm::Window(*this);
    }
    window->present();
}

void Application::setup_actions() {
    auto action_quit = Gio::SimpleAction::create("quit");
    action_quit->signal_activate().connect([this](const Glib::VariantBase&) {
        auto* win = dynamic_cast<Window*>(Gtk::Application::get_active_window());
        if (win) {
            win->hide();
        }
    });
    add_action(action_quit);
}

void Application::setup_accels() {
    set_accel_for_action("app.quit", "<Primary>q");
}

} // namespace ViTerm
