#pragma once

namespace ndbl
{
    typedef int NodeType;
    enum NodeType_
    {
        // enum is used to index arrays, must start at 0 with no gaps

        NodeType_DEFAULT = 0,
        NodeType_BLOCK_CONDITION,
        NodeType_BLOCK_FOR_LOOP,
        NodeType_BLOCK_WHILE_LOOP,
        NodeType_BLOCK_SCOPE,
        NodeType_VARIABLE,
        NodeType_VARIABLE_REF,
        NodeType_LITERAL,
        NodeType_FUNCTION,
        NodeType_OPERATOR,
        NodeType_EMPTY_INSTRUCTION,

        NodeType_COUNT,
    };
}