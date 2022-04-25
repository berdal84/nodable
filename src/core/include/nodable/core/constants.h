#pragma once

namespace ndbl
{
    /*
     * some constants to define specific member names
     */
    static constexpr const char* k_this_member_name                   = "this";             // member to "this Node".
    static constexpr const char* k_value_member_name                  = "value";            // member to a Member holding the value of "this Node".
    static constexpr const char* k_lh_value_member_name               = "L value";          // left handed value
    static constexpr const char* k_rh_value_member_name               = "R value";          // right handed value
    static constexpr const char* k_func_arg_member_name_prefix        = "arg_";             // function argument
    static constexpr const char* k_forloop_initialization_member_name = "initialization";
    static constexpr const char* k_forloop_iteration_member_name      = "iteration";
    static constexpr const char* k_condition_member_name              = "condition";
}