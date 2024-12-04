#pragma once

namespace ndbl
{
    typedef int ASTNodeType;
    enum ASTNodeType_
    {
        // enum is used to index arrays, must start at 0 with no gaps

        ASTNodeType_DEFAULT = 0,
        ASTNodeType_ENTRY_POINT,
        ASTNodeType_BLOCK_IF,
        ASTNodeType_BLOCK_FOR_LOOP,
        ASTNodeType_BLOCK_WHILE_LOOP,
        ASTNodeType_VARIABLE,
        ASTNodeType_VARIABLE_REF,
        ASTNodeType_LITERAL,
        ASTNodeType_FUNCTION,
        ASTNodeType_OPERATOR,
        ASTNodeType_EMPTY_INSTRUCTION,

        ASTNodeType_COUNT,
    };
}