#pragma once

#include "ASTScopeInterface.h"
#include "ASTToken.h"
#include "ASTNodeComponent.h"

namespace ndbl
{
    class ASTScope : public ASTNodeComponent, public ASTScopeInterface
    {
    public:
        ASTScope();
        ~ASTScope() = default;

        ASTToken token_begin;
        ASTToken token_end;

        std::vector<ASTNode*>   get_last_instructions_rec();
        std::vector<ASTNode*>&  get_last_instructions_rec(std::vector<ASTNode*>& _out) override ;
        bool                    has_no_variable()const override { return m_variables.empty(); }
        void                    add_variable(ASTVariableNode*) override ;
        void                    remove_variable(ASTVariableNode *_variable)override;
        size_t                  remove_all_variables() override;
        ASTVariableNode*        find_variable(const std::string& _identifier) override ;
        const VariableNodeVec&  variables()const override { return m_variables; };

    private:
        VariableNodeVec m_variables;

        REFLECT_DERIVED_CLASS()
    };
}
