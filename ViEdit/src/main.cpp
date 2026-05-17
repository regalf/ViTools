#include "application.hpp"

int main(int argc, char* argv[]) {
    auto app = ViEdit::Application::create();
    return app->run(argc, argv);
}
