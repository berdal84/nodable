#pragma once

namespace ndbl
{
    /*
     * some constants to define specific property names
     */
    static constexpr const char* k_this_property_name                   = "this";             // Refers to node's this address.
    static constexpr const char* k_value_property_name                  = "value";            // Refers to node's value.
    static constexpr const char* k_lh_value_property_name               = "L value";          // Refers to node's left-handed value
    static constexpr const char* k_rh_value_property_name               = "R value";          // Refers to node's right-handed value
    static constexpr const char* k_interative_init_property_name        = "initialization";   // Refers to iterative node's initialization instruction.
    static constexpr const char* k_interative_iter_property_name        = "iteration";        // Refers to iterative node's interative instruction.
    static constexpr const char* k_conditional_cond_property_name       = "condition";        // Refers to conditional node's condition.
}