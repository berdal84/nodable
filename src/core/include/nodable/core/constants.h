#pragma once

namespace ndbl
{
    /*
     * some constants to define specific member names
     */
    static constexpr const char* k_this_member_name                   = "this";             // Refers to node's this address.
    static constexpr const char* k_value_member_name                  = "value";            // Refers to node's value.
    static constexpr const char* k_lh_value_member_name               = "L value";          // Refers to node's left-handed value
    static constexpr const char* k_rh_value_member_name               = "R value";          // Refers to node's right-handed value
    static constexpr const char* k_func_arg_member_name_prefix        = "arg_";             // Node's function argument prefix
    static constexpr const char* k_interative_init_member_name        = "initialization";   // Refers to iterative node's initialization instruction.
    static constexpr const char* k_interative_iter_member_name        = "iteration";        // Refers to iterative node's interative instruction.
    static constexpr const char* k_conditional_cond_member_name       = "condition";        // Refers to conditional node's condition.
}