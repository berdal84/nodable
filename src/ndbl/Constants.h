#pragma once

#include "bdc/String.hpp"

namespace ndbl
{
    /*
     * some constants to define specific property names
     */
    constexpr static auto DEFAULT_PROPERTY        = bdc::string("value");            // Refers to node's default property, usually its output.
    constexpr static auto LEFT_VALUE_PROPERTY     = bdc::string("L value");          // Refers to node's left-handed value
    constexpr static auto RIGHT_VALUE_PROPERTY    = bdc::string("R value");          // Refers to node's right-handed value
    constexpr static auto INITIALIZATION_PROPERTY = bdc::string("initialization");   // Refers to iterative node's initialization instruction.
    constexpr static auto ITERATION_PROPERTY      = bdc::string("iteration");        // Refers to iterative node's interative instruction.
    constexpr static auto CONDITION_PROPERTY      = bdc::string("condition");        // Refers to conditional node's condition.
}