#include "ndbl/gui/index.h"

int main(int argc, char *argv[])
{
    ndbl::init_reflection_with_gui();

    TOOLS_try
    {
        ndbl::App_State state;
        nodable_init(&state);
        nodable_run(&state);
        nodable_deinit(&state);
    }
    TOOLS_catch
    return 0;
}
