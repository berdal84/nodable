#include "ndbl/gui/index.h"

int main(int argc, char *argv[])
{
    ndbl::init_with_gui();

    TOOLS_try
    {
        ndbl::app_init();
        ndbl::app_run();
        ndbl::app_shutdown();
    }
    TOOLS_catch
    return 0;
}
