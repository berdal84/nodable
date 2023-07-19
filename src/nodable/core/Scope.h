#pragma once

#include "core/IScope.h"
#include "core/Token.h"
#include "Component.h"

namespace ndbl
{
    class Scope : public Component, public IScope
    {
    public:
        Scope();
        ~Scope(){}

        Node*                   get_last_code_block();
        void                    get_last_instructions_rec(std::vector<InstructionNode *> &_out) override ;

        bool                    has_no_variable()const override { return m_variables.empty(); }
        void                    add_variable(VariableNode*) override ;
        void                    remove_variable(VariableNode *_variable)override;
        size_t                  remove_all_variables() override;
        VariableNode*           find_variable(const std::string &_name) override ;
        const VariableNodeVec&  get_variables()const override { return m_variables; };

    public:
        Token token_begin;
        Token token_end;
    private:
        VariableNodeVec        m_variables;

        REFLECT_DERIVED_CLASS()
    };
}
