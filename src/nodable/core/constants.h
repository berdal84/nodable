#pragma once

namespace ndbl
{
    /*
     * some constants to define specific property names
     */
    static const char* THIS_PROPERTY = "this";             // Refers to node's this address.
    static const char* VALUE_PROPERTY = "value";            // Refers to node's value.
    static const char* ROOT_PROPERTY = "root";
    static const char* LEFT_VALUE_PROPERTY = "L value";          // Refers to node's left-handed value
    static const char* RIGHT_VALUE_PROPERTY = "R value";          // Refers to node's right-handed value
    static const char* INITIALIZATION_PROPERTY = "initialization";   // Refers to iterative node's initialization instruction.
    static const char* ITERATION_PROPERTY = "iteration";        // Refers to iterative node's interative instruction.
    static const char* CONDITION_PROPERTY = "condition";        // Refers to conditional node's condition.
}