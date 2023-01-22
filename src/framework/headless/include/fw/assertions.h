#pragma once

#include <fw/Log.h> // to flush before to assert/throw

// Assertion
#ifndef _NDBL_ASSERT
#define _NDBL_ASSERT(expression) \
    LOG_FLUSH(); \
    assert((expression));
#endif

// Expectation throwing an exception
#ifndef _NDBL_EXPECT
#define _NDBL_EXPECT(expression, message_if_fails )\
    if(!(expression)) { LOG_FLUSH(); throw std::runtime_error(message_if_fails); }
#endif

// Assert shortcut
#if NOEXCEPT
#   include <cassert>
#   define NDBL_ASSERT(expression)          _NDBL_ASSERT( expression )
#   define NDBL_EXPECT(expression, message) _NDBL_ASSERT( expression )
#else
#   include <stdexcept>
#   include <cassert>
#   define NDBL_ASSERT(expression)          _NDBL_EXPECT( (expression), "Assertion failed: " #expression" is false" )
#   define NDBL_EXPECT(expression, message) _NDBL_EXPECT( (expression), message )
#endif
