#pragma once

#include <nodable/Token.h>
#include <nodable/Node.h> // base class
#include <nodable/AbstractCodeBlock.h> // interface
#include <nodable/AbstractConditionalStruct.h> // interface

namespace Nodable
{
    // forward declarations
    class ScopeNode;

    /**
     * @brief Class to represent a conditional structure ( IF/ELSE )
     */
    class ConditionalStructNode
            : public Node // base class
            , public AbstractCodeBlock // interface
            , public AbstractConditionalStruct // interface
    {
    public:
        ConditionalStructNode();
        ~ConditionalStructNode() = default;

        inline void            set_token_if(Token *token) { m_token_if = token; }
        inline void            set_token_else(Token *token) { m_token_else = token; }
        inline const Token*    get_token_if()const   { return m_token_if; }
        inline const Token*    get_token_else()const   { return m_token_else; }

        // override AbstractConditionalStruct
        void                   set_condition(Member *_value) const override { get_condition()->set(_value); }
        ScopeNode*   get_condition_true_branch()const override;
        ScopeNode*   get_condition_false_branch()const override;
        Member*                get_condition()const override { return m_props.get("condition"); }

        // override AbstractCodeBlock
        void                   get_last_instructions(std::vector<InstructionNode *> &out) override;
        inline void            clear() override { set_token_if(nullptr); set_token_else(nullptr);};

    private:
        Token* m_token_if;
        Token* m_token_else;

        REFLECT_DERIVED(ConditionalStructNode)
         REFLECT_EXTENDS(Node)
         REFLECT_EXTENDS(AbstractConditionalStruct)
         REFLECT_EXTENDS(AbstractCodeBlock)
        REFLECT_END
    };
}