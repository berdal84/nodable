#include "AppExample.h"

using namespace fw;

int main(int argc, char *argv[])
{
    // Create a framework configuration, and override some fields ...
    Config conf;
    conf.app_window_label = "framework-example - (based on framework-gui library)";

    // Instantiate the application using the predefined configuration
    AppExample app{conf};

    // Run the main loop until user closes the app or a crash happens...
    return app.main(argc, argv);
}
