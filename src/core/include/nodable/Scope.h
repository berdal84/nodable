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

        inline std::shared_ptr<Token> get_begin_scope_token() const { return m_begin_scope_token; }
        inline std::shared_ptr<Token> get_end_scope_token() const { return m_end_scope_token; }
        inline void             set_begin_scope_token(std::shared_ptr<Token> token) { m_begin_scope_token = token; }
        inline void             set_end_scope_token(std::shared_ptr<Token> token) { m_end_scope_token = token; }

        void                    add_variable(VariableNode*) override ;
        VariableNode*           find_variable(const std::string &_name) override ;
        const VariableNodes&    get_variables()const override { return m_variables; };

    private:

        VariableNodes m_variables;
        std::shared_ptr<Token> m_begin_scope_token;
        std::shared_ptr<Token> m_end_scope_token;

        /** Reflect class */
        R_DERIVED(Scope)
            R_EXTENDS(Component)
            R_EXTENDS(IScope)
        R_END
    };
}
