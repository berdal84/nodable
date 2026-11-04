#pragma once
#include "bdc/Allocators.hpp"
#include "Log.h"

#ifndef NDBL_ASSERTIONS_ENABLE
#define NDBL_ASSERTIONS_ENABLE true // When false, any ASSERT/VERIFY macros are disable
#endif

#if NDBL_ASSERTIONS_ENABLE // ---------------------------------------------------------------------------

    #define NDBL_NOEXCEPT NDBL_RELEASE // In release, we disable exceptions and fallback on regular asserts

    #if NDBL_NOEXCEPT // --------------------------------------------------------------------------------

        #include <cassert>
        #define NDBL_ASSERT(expression)          assert( (expression) )
        #define NDBL_VERIFY(expression, message) NDBL_ASSERT( expression )

    #else // NDBL_NOEXCEPT ------------------------------------------------------------------------------

        #include <exception> // for std::runtime_error

        #define NDBL_VERIFY_(expression, message_if_fails, throw_on_failure )\
        if( !(expression) ) \
        { \
            BDC_PRINT_STACKTRACE(); \
            printf("VERIFY: %s was evaluated false. Message: %s\n", #expression, message_if_fails ); \
            flush(); \
            if ( throw_on_failure ) \
                throw std::runtime_error(message_if_fails); \
            assert( false ); \
        }

        #define NDBL_ASSERT(expression)          NDBL_VERIFY_( (expression), "Assertion failed: " #expression" is false", false )
        #define NDBL_VERIFY(expression, message) NDBL_VERIFY_( (expression), message                                    , true )

        #endif // !NDBL_NOEXCEPT

        // DEBUG SPECIFIC
        #ifdef NDBL_DEBUG
        #define NDBL_ASSERT_DEBUG_ONLY(expression) NDBL_ASSERT(expression)
        #else
        #define NDBL_ASSERT_DEBUG_ONLY(expression)
    #endif // NDBL_NOEXCEPT -----------------------------------------------------------------------------

#else // NDBL_ASSERTIONS_ENABLE -------------------------------------------------------------------------

    // Disable the macros completely
    #define NDBL_ASSERT(...)
    #define NDBL_VERIFY(...)

#endif // NDBL_ASSERTIONS_ENABLE -----------------------------------------------------------------------

#define NDBL_UNREACHABLE( ... ) \
do { \
    BDC_PRINT_STACKTRACE(); \
    printf("UNREACHABLE: %s\n", #__VA_ARGS__); \
    flush(); \
    assert( false ); \
} while(0)

#define NDBL_TODO( message ) \
do { \
    BDC_PRINT_STACKTRACE(); \
    printf("TODO: %s\n", #message); \
    flush(); \
    assert( false ); \
} while(0)

