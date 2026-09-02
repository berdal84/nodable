#pragma once
#include "tools/core/Log.h"
#include "bdc/Allocators.hpp"

#ifndef TOOLS_ASSERTIONS_ENABLE
#define TOOLS_ASSERTIONS_ENABLE true // When false, any ASSERT/VERIFY macros are disable
#endif

#if TOOLS_ASSERTIONS_ENABLE // ---------------------------------------------------------------------------

    #define TOOLS_NOEXCEPT TOOLS_RELEASE // In release, we disable exceptions and fallback on regular asserts

    #if TOOLS_NOEXCEPT // --------------------------------------------------------------------------------

        #include <cassert>
        #define ASSERT(expression)          assert( (expression) )
        #define VERIFY(expression, message) ASSERT( expression )

    #else // TOOLS_NOEXCEPT ------------------------------------------------------------------------------

        #include <exception> // for std::runtime_error

        #ifdef VERIFY_
            static_assert(false, "VERIFY_ is reserved for tools, it should not be defined here.")
        #endif

        #define VERIFY_(expression, message_if_fails, throw_on_failure )\
        if( !(expression) ) \
        { \
            BDC_PRINT_STACKTRACE(); \
            printf("VERIFY: %s was evaluated false. Message: %s\n", #expression, message_if_fails ); \
            tools::flush(); \
            if ( throw_on_failure ) \
                throw std::runtime_error(message_if_fails); \
            assert( false ); \
        }

        #define ASSERT(expression)          VERIFY_( (expression), "Assertion failed: " #expression" is false", false )
        #define VERIFY(expression, message) VERIFY_( (expression), message                                    , true )

        #endif // !TOOLS_NOEXCEPT

        // DEBUG SPECIFIC
        #ifdef TOOLS_DEBUG
        #define ASSERT_DEBUG_ONLY(expression) ASSERT(expression)
        #else
        #define ASSERT_DEBUG_ONLY(expression)
    #endif // TOOLS_NOEXCEPT -----------------------------------------------------------------------------

#else // TOOLS_ASSERTIONS_ENABLE -------------------------------------------------------------------------

    // Disable the macros completely
    #define ASSERT(...)
    #define VERIFY(...)

#endif // TOOLS_ASSERTIONS_ENABLE -----------------------------------------------------------------------

#define TOOLS_UNREACHABLE( ... ) \
do { \
    BDC_PRINT_STACKTRACE(); \
    printf("UNREACHABLE: %s\n", #__VA_ARGS__); \
    tools::flush(); \
    assert( false ); \
} while(0)

#define TODO( message ) \
do { \
    BDC_PRINT_STACKTRACE(); \
    printf("TODO: %s\n", #message); \
    tools::flush(); \
    assert( false ); \
} while(0)

