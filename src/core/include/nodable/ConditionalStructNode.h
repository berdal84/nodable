#pragma once

#include <nodable/Token.h> // base class
#include <nodable/Node.h> // base class
#include <nodable/AbstractCodeBlock.h> // base class

namespace Nodable
{
    // forward declarations
    class ScopedCodeBlockNode;

    /**
     * @brief Class to represent a conditional structure ( IF/ELSE )
     */
    class ConditionalStructNode: public Node, public AbstractCodeBlock {
    public:
        ConditionalStructNode();
        ~ConditionalStructNode() = default;

        inline void set_condition(Member *_value) const { get_condition()->set(_value); }
        inline void set_token_if(Token *token) { m_token_if = token; }
        inline void set_token_else(Token *token) { m_token_else = token; }

        ScopedCodeBlockNode*   get_if_branch()const;
        ScopedCodeBlockNode*   get_else_branch()const;
        inline Member*         get_condition()const { return m_props.get("condition"); }
        inline const Token*    get_token_if()const   { return m_token_if; }
        inline const Token*    get_token_else()const   { return m_token_else; }
        inline void            clear() override { set_token_if(nullptr); set_token_else(nullptr);};
        bool                   has_instructions() const override;
        InstructionNode*       get_first_instruction() const override;
        void                   get_last_instructions(std::vector<InstructionNode *> &out) override;
    private:
        Token* m_token_if;
        Token* m_token_else;

        REFLECT_DERIVED(ConditionalStructNode)
         REFLECT_EXTENDS(Node)
        REFLECT_END
    };
}