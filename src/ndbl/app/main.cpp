#include "ndbl/gui/index.h"

int main(int argc, char *argv[])
{
    ndbl::init_reflection_with_gui();

    TOOLS_try
    {
        ndbl::Nodable app;
        app.init();
        app.run();
        app.shutdown();
    }
    TOOLS_catch
    return 0;
}
