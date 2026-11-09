#include "Nodable.h"
#include "bdc/Signal_Handler.hpp"

int main(int argc, char *argv[])
{
    BDC_INSTALL_SIGNAL_HANDLER(); // to catch signals (ex: SIGSEGV, SIGILL, etc.)

    // program
    ndbl::app_init();
    ndbl::app_run();
    ndbl::app_shutdown();

    std::exit(0);
}
