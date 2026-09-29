#include <iostream>

#include "Application.h"

int main(int argc, char* argv[]) {
    Application app;

    if (argc > 2) {
        std::cerr << "Usage: raytracer <model_name>\n";
        return -1;
    }

    std::string modelName = argv[1];

    if (!app.initialize(modelName)) {
        return -1;
    }

    std::cout << "Finished Initialization." << std::endl;

    app.run();

    return 0;
}