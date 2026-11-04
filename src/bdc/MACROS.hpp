#pragma once

#ifdef BDC_ENABLE_LOGS
    #define BDC_LOG(...) printf(__VA_ARGS__)
    #define BDC_LOG_DEBUG(...) printf(__VA_ARGS__)
#else
    #define BDC_LOG(...)
    #define BDC_LOG_DEBUG(...)
#endif

#pragma once
#ifndef BDC_ASSERTIONS_ENABLE
#define BDC_ASSERTIONS_ENABLE true
#endif

#if BDC_ASSERTIONS_ENABLE

    #include "bdc/Stacktrace.hpp"
    #include <cassert>
    #include <iostream> // for std::flush, std::cout

    #define BDC_ASSERT(expression, ... ) \
        if( !(expression) ) \
        { \
            BDC_PRINT_STACKTRACE(); \
            printf("NDBL_ASSERT: Expression '%s' is false. \n." __VA_OPT__(" Reason: " __VA_ARGS__ ), #expression ); \
            std::cout << std::flush; \
            assert( false ); \
        }


#else // BDC_ASSERTIONS_ENABLE

    #define BDC_ASSERT(...)

#endif // BDC_ASSERTIONS_ENABLE





