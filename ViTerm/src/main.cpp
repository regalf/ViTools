#include "application.hpp"

int main(int argc, char* argv[]) {
    auto app = ViTerm::Application::create();
    return app->run(argc, argv);
}
