#pragma once

#include <cassert>
#include <nodable/core/Log.h>

// Ensure to flush log before to assert
#define NODABLE_ASSERT(expression) LOG_FLUSH(); assert((expression));
