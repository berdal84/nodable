#pragma once

#ifndef TOOLS_ASSERTIONS_ENABLE
#define TOOLS_ASSERTIONS_ENABLE true
#endif

#if TOOLS_ASSERTIONS_ENABLE

#define TOOLS_NOEXCEPT NOEXCEPT

// TOOLS_ASSERTIONS_ENABLE ON
//----------------------------

#include "log.h" // to flush before to assert/throw
#include <cassert>

// Exception OFF
//--------------

#if TOOLS_NOEXCEPT

#ifdef ASSERT_
static_assert(false, "ASSERT_ is reserved for tools, it should not be defined here.");
#endif

#define ASSERT_(expression)         LOG_FLUSH(); assert((expression))
#define ASSERT(expression)          ASSERT_( expression );
#define VERIFY(expression, message) ASSERT_( (expression) && message );

#else // TOOLS_NOEXCEPT

// Exception ON
//-------------

#include "Exceptions.h"

#ifdef VERIFY_
static_assert(false, "VERIFY_ is reserved for tools, it should not be defined here.")
#endif

#define VERIFY_(expression, message_if_fails )\
if(!(expression)) { LOG_FLUSH() throw tools::runtime_error(message_if_fails); }

#define ASSERT(expression) VERIFY_( (expression), "Assertion failed: " #expression" is false" )
#define VERIFY(expression, message) VERIFY_( (expression), message )

#endif // !TOOLS_NOEXCEPT

#else // TOOLS_ASSERTIONS_ENABLE

// TOOLS_ASSERTIONS_ENABLE OFF
//----------------------------

// Disable the macros completely
#define ASSERT(...)
#define EXPECT(...)

#endif // TOOLS_ASSERTIONS_ENABLE
