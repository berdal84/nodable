#pragma once

#include "Node/ScopedCodeBlockNode.h" // base class

namespace Nodable
{
    /**
     * @brief Class to represent a conditional structure ( IF/ELSE )
     */
    class ConditionalStructNode: public ScopedCodeBlockNode {
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

    // reflect class using mirror
    MIRROR_CLASS(ConditionalStructNode)
    (
        MIRROR_PARENT(ScopedCodeBlockNode)
    )
    };
}
