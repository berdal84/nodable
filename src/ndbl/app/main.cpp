#include "ndbl/gui/index.h"

int main(int argc, char *argv[])
{
    ndbl::init_reflection_with_gui();

    TOOLS_try
    {
        ndbl::NodableState state;
        nodable_init(&state);
        nodable_run(&state);
        nodable_shutdown(&state);
    }
    TOOLS_catch
    return 0;
}
