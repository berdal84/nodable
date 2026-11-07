#pragma once

#ifdef __EMSCRIPTEN__
    #include <emscripten/em_js.h>
    #include <iostream>

    namespace bdc
    {
        EM_JS(void, print_stacktrace, (), {
            console.log(stackTrace());
        });
    }

    #define BDC_PRINT_STACKTRACE() \
        bdc::print_stacktrace()
#else
    #include <ostream>    // for std::cout, std::endl
    #include <stacktrace> // for std::stacktrace
    #define BDC_PRINT_STACKTRACE() \
        std::stacktrace st = std::stacktrace::current(); \
        std::cout << st << std::endl
#endif 