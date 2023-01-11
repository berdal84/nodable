#pragma once

#include <nodable/core/Log.h> // to flush before to assert/throw

// Assertion
#define _NDBL_ASSERT_NOEXCEPT(expression) \
    LOG_FLUSH(); \
    assert((expression));

// Assertion throwing an exception
#define _NDBL_ASSERT(expression, message_if_fails )\
    LOG_FLUSH(); \
    if(!(expression)) throw std::runtime_error(message_if_fails);

// Assert shortcut
#if NOEXCEPT
#   include <cassert>
#   define NDBL_ASSERT(expression)             _NDBL_ASSERT_NOEXCEPT( expression )
#   define NDBL_ASSERT_EX(expression, message) _NDBL_ASSERT_NOEXCEPT( expression )
#else
#   include <stdexcept>
#   include <cassert>
#   define NDBL_ASSERT(expression)             _NDBL_ASSERT( (expression), "assertion failed" )
#   define NDBL_ASSERT_EX(expression, message) _NDBL_ASSERT( (expression), message )
#endif

#undef _NDBL_ASSERT_NOEXCEPT
#undef _NDBL_ASSERT_EXCEPT
