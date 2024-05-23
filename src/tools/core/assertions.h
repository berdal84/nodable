#pragma once

#include "log.h" // to flush before to assert/throw

// Assertion is stopping the program when expression is false
#ifndef ASSERT_
#define ASSERT_(expression) \
    LOG_FLUSH(); \
    assert((expression));
#endif

#if NOEXCEPT
#   include <cassert>
#   define ASSERT(expression)          ASSERT_( expression )
#   define EXPECT(expression, message) ASSERT_( expression )
#else
#   include <stdexcept>
#   include <cassert>

// Expect is throwing an exception when expression is false
#   ifndef EXPECT_
#   define EXPECT_(expression, message_if_fails )\
        if(!(expression)) { LOG_FLUSH(); throw std::runtime_error(message_if_fails); }
#   endif
#   define ASSERT(expression) EXPECT_( (expression), "Assertion failed: " #expression" is false" )
#   define EXPECT(expression, message) EXPECT_( (expression), message )
#endif
