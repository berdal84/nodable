#pragma once

#include "Stacktrace.hpp"
#include <csignal>

#define BDC_INSTALL_SIGNAL_HANDLER() \
static bdc::Signal_Handler_Initializer bdc_signal_handler_initializer{ bdc::signal_handler }

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

    struct Signal_Handler_Initializer
    {
        constexpr Signal_Handler_Initializer(auto _signal_handler)
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
    };

} // namespace bdc