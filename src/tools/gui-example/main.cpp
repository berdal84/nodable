#include "tools/gui/index.h"

// unity build
#include "tools/gui-example/AppExample.cpp"
#include "tools/gui-example/AppExampleView.cpp"
// unity build (end)

using namespace tools;

int main(int argc, char *argv[])
{
    tools::init_reflection();

    TOOLS_try
    {
        // Instantiate the application using the predefined configuration
        AppExample app;
        app.init();
        app.run();
        app.shutdown();
    }
    TOOLS_catch
    return 0;
}
