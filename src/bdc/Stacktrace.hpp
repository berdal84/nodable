#pragma once
#include "MACROS.hpp"

#ifdef __EMSCRIPTEN__
    #include <emscripten/em_asm.h> // for EM_ASM, console.log, console.debug
#else
    #include <stacktrace> // for std::stacktrace
#endif

namespace bdc
{
    inline void log_stacktrace()
    {
        #ifdef __EMSCRIPTEN__
            EM_ASM({ console.log(stackTrace()); });
        #else
            BDC_LOG("{}", std::stacktrace::current());
        #endif
    }

    inline void debug_stacktrace()
    {
        #ifdef __EMSCRIPTEN__
            EM_ASM({ console.debug(stackTrace()); });
        #else
            BDC_LOG_DEBUG("{}", std::stacktrace::current());
        #endif
    }

} // namespace ndbl