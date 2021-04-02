#pragma once

#include "Node/CodeBlockNode.h" // base class

namespace Nodable
{
    /**
     * @brief Class to represent a conditional structure ( IF/ELSE )
     * TODO: add getter/setters for if/else scopes
     */
    class ConditionalStructNode: public CodeBlockNode {
    public:
        ConditionalStructNode();
        ~ConditionalStructNode() = default;
        [[nodiscard]] Member* getCondition()const
        {
            return get("condition");
        }

        void setCondition(Member* _value)
        {
            get("condition")->set(_value);
        };
        Token* token_if;
        Token* token_else;

    // reflect class using mirror
    MIRROR_CLASS(ConditionalStructNode)
    (
        MIRROR_PARENT(CodeBlockNode)
    )
    };
}
