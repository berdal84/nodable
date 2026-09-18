#include "ndbl/gui/Nodable.h"

int main(int argc, char *argv[])
{
    NDBL_try
    {
        ndbl::app_init();
        ndbl::app_run();
        ndbl::app_shutdown();
    }
    NDBL_catch

    return 0;
}
