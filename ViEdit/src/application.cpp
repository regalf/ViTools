#include "application.hpp"
#include "window.hpp"

namespace ViEdit {

Glib::RefPtr<Application> Application::create() {
    return Glib::make_refptr_for_instance<Application>(new Application());
}

Application::Application() : Gtk::Application("com.vitools.viedit", Gio::Application::Flags::HANDLES_OPEN) {
}

Application::~Application() = default;

void Application::on_startup() {
    Gtk::Application::on_startup();
    setup_actions();
}

void Application::on_activate() {
    auto* app_window = get_active_window();
    if (!app_window) {
        auto window = ViEdit::Window::create(*this);
        add_window(*window);
        window->present();
    }
}

void Application::setup_actions() {
    add_action("quit", sigc::mem_fun(*this, &Application::on_activate));
}

} // namespace ViEdit
