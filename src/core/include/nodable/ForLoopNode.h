#pragma once

#include <nodable/Token.h>
#include <nodable/Node.h> // base class
#include <nodable/AbstractCodeBlock.h> // interface
#include <nodable/AbstractConditionalStruct.h> // interface

namespace Nodable
{
    // forward declarations
    class ScopedCodeBlockNode;

    /**
     * @brief Class to represent a conditional iterative structure: for( init_state, condition_expr, iterate_expr ) { ... }
     */
    class ForLoopNode
            : public Node
            , public AbstractCodeBlock
            , public AbstractConditionalStruct {
    public:
        ForLoopNode();
        ~ForLoopNode() = default;

        inline void            set_token_for(Token* _token) { m_token_for = _token; }
        inline const Token*    get_token_for()const   { return m_token_for; }
        Member*                get_init_expr()const { return m_props.get("init"); }
        Member*                get_iter_expr()const { return m_props.get("iter"); }

        // override AbstractConditionalStruct
        void                   set_condition(Member *_value) const override { get_condition()->set(_value); }
        Member*                get_condition()const override { return m_props.get("condition"); }
        ScopedCodeBlockNode*   get_condition_true_branch()const override;
        ScopedCodeBlockNode*   get_condition_false_branch()const override;

        // override AbstractCodeBlock
        bool                   has_instructions() const override;
        InstructionNode*       get_first_instruction() const override;
        void                   get_last_instructions(std::vector<InstructionNode *> &out) override;
        inline void            clear() override { set_token_for(nullptr); };

    private:
        Token* m_token_for;

        REFLECT_DERIVED(ForLoopNode)
            REFLECT_EXTENDS(Node)
            REFLECT_EXTENDS(AbstractConditionalStruct)
            REFLECT_EXTENDS(AbstractCodeBlock)
        REFLECT_END
    };
}
