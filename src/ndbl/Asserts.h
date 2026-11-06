#pragma once
#ifndef NDBL_ASSERTIONS_ENABLE
#define NDBL_ASSERTIONS_ENABLE true
#endif

#if NDBL_ASSERTIONS_ENABLE

    #include "bdc/Stacktrace.hpp"
    #include <cassert>
    #include <iostream> // for std::flush, std::cout

    #define NDBL_ASSERT(expression, ... ) \
        if( !(expression) ) \
        { \
            BDC_PRINT_STACKTRACE(); \
            printf("NDBL_ASSERT: Expression '%s' is false. \n." __VA_OPT__(" Reason: " __VA_ARGS__ ), #expression ); \
            std::cout << std::flush; \
            assert( false ); \
        }

    #ifdef NDBL_DEBUG
        #define NDBL_ASSERT_DEBUG_ONLY( ... ) \
            NDBL_ASSERT(__VA_ARGS__, "(debug only)")
    #else
        #define NDBL_ASSERT_DEBUG_ONLY( ... ) // NDBL_ASSERT_DEBUG_ONLY was removed, NDBL_DEBUG is false.
    #endif

    #define NDBL_TODO( message ) \
        do { \
            BDC_PRINT_STACKTRACE(); \
            printf("NDBL_TODO: %s\n", #message); \
            std::cout << std::flush; \
            assert( false ); \
        } while(0)

    #define NDBL_UNREACHABLE( ... ) \
    do { \
        BDC_PRINT_STACKTRACE(); \
        printf("NDBL_UNREACHABLE: %s\n", #__VA_ARGS__); \
        std::cout << std::flush; \
        assert( false ); \
    } while(0)

#else // NDBL_ASSERTIONS_ENABLE

    #define NDBL_ASSERT(...)
    #define NDBL_TODO(...)
    #define NDBL_UNREACHABLE( ... )

#endif // NDBL_ASSERTIONS_ENABLE




