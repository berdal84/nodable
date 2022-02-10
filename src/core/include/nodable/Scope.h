#pragma once

#include <nodable/Component.h>
#include <nodable/IScope.h>
#include <nodable/Token.h>

namespace Nodable
{
    class Scope : public Component, public IScope
    {
    public:
        Scope();
        ~Scope(){}

        bool                    update() override { return true; };
        void                    clear() override;
        Node*                   get_last_code_block();
        void                    get_last_instructions(std::vector<InstructionNode *> &out) override ;

        inline const Token*     get_begin_scope_token() const { return m_begin_scope_token; }
        inline const Token*     get_end_scope_token() const { return m_end_scope_token; }
        inline void             set_begin_scope_token(Token *token) { m_begin_scope_token = token; }
        inline void             set_end_Scope_token(Token *token) { m_end_scope_token = token; }

        void                    add_variable(VariableNode*) override ;
        VariableNode*           find_variable(const std::string &_name) override ;
        const VariableNodes&    get_variables()const override { return m_variables; };

    private:

        VariableNodes m_variables;
        Token*        m_begin_scope_token;
        Token*        m_end_scope_token;

        /** Reflect class */
        REFLECT_DERIVED(Scope)
            REFLECT_EXTENDS(Component)
            REFLECT_EXTENDS(IScope)
        REFLECT_END
    };
}
