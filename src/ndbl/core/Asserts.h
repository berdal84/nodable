#pragma once
#include "ndbl/core/Log.h"
#include "bdc/Allocators.hpp"

#ifndef NDBL_ASSERTIONS_ENABLE
#define NDBL_ASSERTIONS_ENABLE true // When false, any ASSERT/VERIFY macros are disable
#endif

#if NDBL_ASSERTIONS_ENABLE // ---------------------------------------------------------------------------

    #define NDBL_NOEXCEPT NDBL_RELEASE // In release, we disable exceptions and fallback on regular asserts

    #if NDBL_NOEXCEPT // --------------------------------------------------------------------------------

        #include <cassert>
        #define ASSERT(expression)          assert( (expression) )
        #define VERIFY(expression, message) ASSERT( expression )

    #else // NDBL_NOEXCEPT ------------------------------------------------------------------------------

        #include <exception> // for std::runtime_error

        #ifdef VERIFY_
            static_assert(false, "VERIFY_ is reserved for tools, it should not be defined here.")
        #endif

        #define VERIFY_(expression, message_if_fails, throw_on_failure )\
        if( !(expression) ) \
        { \
            BDC_PRINT_STACKTRACE(); \
            printf("VERIFY: %s was evaluated false. Message: %s\n", #expression, message_if_fails ); \
            flush(); \
            if ( throw_on_failure ) \
                throw std::runtime_error(message_if_fails); \
            assert( false ); \
        }

        #define ASSERT(expression)          VERIFY_( (expression), "Assertion failed: " #expression" is false", false )
        #define VERIFY(expression, message) VERIFY_( (expression), message                                    , true )

        #endif // !NDBL_NOEXCEPT

        // DEBUG SPECIFIC
        #ifdef NDBL_DEBUG
        #define ASSERT_DEBUG_ONLY(expression) ASSERT(expression)
        #else
        #define ASSERT_DEBUG_ONLY(expression)
    #endif // NDBL_NOEXCEPT -----------------------------------------------------------------------------

#else // NDBL_ASSERTIONS_ENABLE -------------------------------------------------------------------------

    // Disable the macros completely
    #define ASSERT(...)
    #define VERIFY(...)

#endif // NDBL_ASSERTIONS_ENABLE -----------------------------------------------------------------------

#define UNREACHABLE( ... ) \
do { \
    BDC_PRINT_STACKTRACE(); \
    printf("UNREACHABLE: %s\n", #__VA_ARGS__); \
    flush(); \
    assert( false ); \
} while(0)

#define TODO( message ) \
do { \
    BDC_PRINT_STACKTRACE(); \
    printf("TODO: %s\n", #message); \
    flush(); \
    assert( false ); \
} while(0)

