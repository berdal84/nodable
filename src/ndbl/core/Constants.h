#pragma once

#include "bdc/String.hpp"

namespace ndbl
{
    /*
     * some constants to define specific property names
     */
    constexpr static const bdc::String DEFAULT_PROPERTY        = "value";            // Refers to node's default property, usually its output.
    constexpr static const bdc::String LEFT_VALUE_PROPERTY     = "L value";          // Refers to node's left-handed value
    constexpr static const bdc::String RIGHT_VALUE_PROPERTY    = "R value";          // Refers to node's right-handed value
    constexpr static const bdc::String INITIALIZATION_PROPERTY = "initialization";   // Refers to iterative node's initialization instruction.
    constexpr static const bdc::String ITERATION_PROPERTY      = "iteration";        // Refers to iterative node's interative instruction.
    constexpr static const bdc::String CONDITION_PROPERTY      = "condition";        // Refers to conditional node's condition.
}