#pragma once

#include <nodable/core/Component.h>
#include <nodable/core/IScope.h>
#include <nodable/core/Token.h>

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

        std::shared_ptr<Token>  get_begin_scope_token() const { return m_begin_scope_token; }
        std::shared_ptr<Token>  get_end_scope_token() const { return m_end_scope_token; }
        void                    set_begin_scope_token(std::shared_ptr<Token> token) { m_begin_scope_token = token; }
        void                    set_end_scope_token(std::shared_ptr<Token> token) { m_end_scope_token = token; }

        bool                    has_no_variable()const override { return m_variables.empty(); }
        void                    add_variable(VariableNode*) override ;
        void                    remove_variable(VariableNode *_variable)override;
        size_t                  remove_all_variables() override;
        VariableNode*           find_variable(const std::string &_name) override ;
        const VariableNodeVec&  get_variables()const override { return m_variables; };

    private:

        VariableNodeVec        m_variables;
        std::shared_ptr<Token> m_begin_scope_token;
        std::shared_ptr<Token> m_end_scope_token;

        /** Reflect class */
        R_DERIVED(Scope)
            R_EXTENDS(Component)
            R_EXTENDS(IScope)
        R_END
    };
}
