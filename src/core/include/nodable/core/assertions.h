#pragma once

#include <nodable/core/Log.h> // to flush before to assert/throw

// Assertion
#define _NODABLE_ASSERT_NOEXCEPT(expression) \
    LOG_FLUSH(); \
    assert((expression));

// Assertion throwing an exception
#define _NODABLE_ASSERT(expression, message_if_fails )\
    LOG_FLUSH(); \
    if(!(expression)) throw std::runtime_error(message_if_fails);

// Assert shortcut
#if NOEXCEPT
#   include <cassert>
#   define NODABLE_ASSERT(expression)             _NODABLE_ASSERT_NOEXCEPT( expression )
#   define NODABLE_ASSERT_EX(expression, message) _NODABLE_ASSERT_NOEXCEPT( expression )
#else
#   include <stdexcept>
#   include <cassert>
#   define NODABLE_ASSERT(expression)             _NODABLE_ASSERT( (expression), "assertion failed" )
#   define NODABLE_ASSERT_EX(expression, message) _NODABLE_ASSERT( (expression), message )
#endif

#undef _NODABLE_ASSERT_NOEXCEPT
#undef _NODABLE_ASSERT_EXCEPT
