#include "tools/gui/index.h"
#include "App_Example.h"

using namespace tools;

int main(int argc, char *argv[])
{
    tools::init_reflection();

    TOOLS_try
    {
        // Instantiate the application using the predefined configuration
        App_Example app;
        app.init();
        app.run();
        app.shutdown();
    }
    TOOLS_catch
    return 0;
}
