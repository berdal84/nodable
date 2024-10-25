#pragma once

#include "IScope.h"
#include "Token.h"
#include "NodeComponent.h"

namespace ndbl
{
    class Scope : public NodeComponent, public IScope
    {
    public:
        Scope();
        ~Scope() = default;

        Token token_begin = {Token_t::parenthesis_open};
        Token token_end   = {Token_t::parenthesis_close};

        std::vector<Node*>      get_last_instructions_rec();
        std::vector<Node*>&     get_last_instructions_rec( std::vector<Node*>& _out) override ;
        bool                    has_no_variable()const override { return m_variables.empty(); }
        void                    add_variable(VariableNode*) override ;
        void                    remove_variable(VariableNode *_variable)override;
        size_t                  remove_all_variables() override;
        VariableNode*           find_variable(const std::string& _identifier) override ;
        const VariableNodeVec&  variables()const override { return m_variables; };

    private:
        VariableNodeVec m_variables;

        REFLECT_DERIVED_CLASS()
    };
}
