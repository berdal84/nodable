#include "AppExample.h"

int main(int argc, char *argv[])
{
    // Create a framework configuration, and override some fields ...
    fw::Config conf;
    conf.app_window_label = "framework-example - (based on framework-gui library)";

    // Instantiate the application using the predefined configuration
    fw::AppExample app{conf};

    // Run the main loop until user closes the app or a crash happens...
    return app.main(argc, argv);
}
