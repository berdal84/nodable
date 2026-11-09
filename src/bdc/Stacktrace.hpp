#pragma once
#include "MACROS.hpp"

#ifdef __EMSCRIPTEN__
    // #include <emscripten/em_js.h>
    // #include <iostream>

    // namespace bdc
    // {
    //     EM_JS(void, print_stacktrace, (), {
    //         console.log(stackTrace());
    //     });
    // }

    #define BDC_LOG_STACKTRACE() \
        BDC_LOG_DEBUG("BDC_LOG_STACKTRACE is not implemented - {}:{}\n", __FILE__, __LINE__);
#else
    #include <stacktrace> // for std::stacktrace
    #define BDC_LOG_STACKTRACE() \
        BDC_LOG_DEBUG("{}", std::stacktrace::current());
#endif 