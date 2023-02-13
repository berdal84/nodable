#pragma once

#include "Log.h" // to flush before to assert/throw

// Assertion is stopping the program when expression is false
#ifndef FW_ASSERT_
#define FW_ASSERT_(expression) \
    LOG_FLUSH(); \
    assert((expression));
#endif

// Expect is throwing an exception when expression is false
#ifndef FW_EXPECT_
#define FW_EXPECT_(expression, message_if_fails )\
    if(!(expression)) { LOG_FLUSH(); throw std::runtime_error(message_if_fails); }
#endif

#if NOEXCEPT
#   include <cassert>
#   define FW_ASSERT(expression)          FW_ASSERT_( expression )
#   define FW_EXPECT(expression, message) FW_ASSERT_( expression )
#else
#   include <stdexcept>
#   include <cassert>
#   define FW_ASSERT(expression) FW_EXPECT_( (expression), "Assertion failed: " #expression" is false" )
#   define FW_EXPECT(expression, message) FW_EXPECT_( (expression), message )
#endif
