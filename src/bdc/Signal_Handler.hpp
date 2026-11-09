#pragma once

#include "Stacktrace.hpp"
#include <csignal>

//
// Add this macro at the beginning of your main()
// Any signal will be caught and a stacktrace will be printed out before the program crashes/abort/exit...
//
#define BDC_INSTALL_SIGNAL_HANDLER() \
bdc::install_signal_handler( bdc::signal_handler )

namespace bdc
{
    //
    // Windows does not have const char* strsignal(int), it is unix only.
    // I preferred defining my own:
    //
    static const char* my_strsignal(int sig)
    {
        switch (sig)
        {
            case SIGABRT:  return "SIGABRT 'Abnormal termination'";
            case SIGFPE:   return "SIGFPE 'Erroneous arithmetic operation'";
            case SIGILL:   return "SIGILL 'Illegal instruction'";
            case SIGINT:   return "SIGINT 'Terminal interrupt'";
            case SIGSEGV:  return "SIGSEGV 'Invalid memory access'";
            default:       return "'Unknown signal'";
        }
    }

    #ifdef _WIN32
    static void signal_handler(int sig, int) // MSVC expects int (*)(int, int), but void(*)(int, int) should be accepted.
    #else
    static void signal_handler(int sig)
    #endif
    {
        BDC_LOG_STACKTRACE();
        BDC_LOG("Caught signal {} ({}). For more info see stacktrace above.\n\n", my_strsignal(sig), sig);
        BDC_LOG_FLUSH();
        
        // re-raise with defaults
        signal(sig, SIG_DFL);
        raise(sig);
    }

    static void install_signal_handler(auto _signal_handler)
    {
    #ifdef _WIN32
        signal(SIGSEGV, _signal_handler);
        signal(SIGFPE,  _signal_handler);
        signal(SIGABRT, _signal_handler);
    #else
        struct sigaction sa{};
        sa.sa_handler = _signal_handler;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;
        sigaction(SIGSEGV, &sa, nullptr);
        sigaction(SIGFPE,  &sa, nullptr);
        sigaction(SIGILL,  &sa, nullptr);
        sigaction(SIGABRT, &sa, nullptr);
        sigaction(SIGBUS,  &sa, nullptr);
        sigaction(SIGTERM, &sa, nullptr);
    #endif
    }

} // namespace bdc