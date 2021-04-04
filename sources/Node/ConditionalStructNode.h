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
        inline Member* getCondition()const { return get("condition"); }
        inline void setCondition(Member* _value){ get("condition")->set(_value);};
        virtual AbstractCodeBlockNode* getNext();
        AbstractCodeBlockNode*         getBranchTrue();
        AbstractCodeBlockNode*         getBranchFalse();
        Token* token_if;
        Token* token_else;

    // reflect class using mirror
    MIRROR_CLASS(ConditionalStructNode)
    (
        MIRROR_PARENT(CodeBlockNode)
    )

    };
}
