#include "Nodable.h"

#include "bdc/Stacktrace.hpp"
#include <csignal>
#include <cstdlib>

static void signal_handler(int sig)
{
    BDC_PRINT_STACKTRACE();
    std::cout << "Fatal signal " << sig << " ('" << strsignal(sig) << "') caught, abording. For more info see stacktrace above."
              << std::endl << std::endl
              << std::flush;

    // re-raise with defaults
    signal(sig, SIG_DFL);
    raise(sig);
}

int main(int argc, char *argv[])
{
    // sigaction
    struct sigaction sa{};
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGSEGV, &sa, nullptr); /* ISO C99: Invalid access to storage.  */
    sigaction(SIGFPE,  &sa, nullptr); /* ISO C99: Erroneous arithmetic operation.  */
    sigaction(SIGILL,  &sa, nullptr); /* ISO C99: Illegal instruction.  */
    sigaction(SIGABRT, &sa, nullptr); /* ISO C99: Abnormal termination.  */
    sigaction(SIGBUS,  &sa, nullptr);
    sigaction(SIGTERM, &sa, nullptr); /* ISO C99: Termination request.  */
    #ifdef NDBL_LINUX
    sigaction(SIGKILL, &sa, nullptr); /* POSIX:   Illegal instruction.  */
    #endif

    // program
    ndbl::app_init();
    ndbl::app_run();
    ndbl::app_shutdown();

    std::exit(0);
}
