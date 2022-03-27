#pragma once
#include <vector>

#include <nodable/core/reflection/R.h>
#include <nodable/core/types.h>

namespace Nodable
{
    // forward declarations
    class InstructionNode;
    class VariableNode;
    class Node;

    class IScope
    {
    public:
        virtual void                 clear() = 0;
        virtual void                 get_last_instructions(std::vector<InstructionNode *> &out) = 0;
        virtual void                 add_variable(VariableNode*) = 0;
        virtual bool                 has_no_variable()const = 0;
        virtual void                 remove_variable(VariableNode*) = 0;
        virtual size_t               remove_all_variables() = 0;
        virtual VariableNode*        find_variable(const std::string &_name) = 0;
        virtual const VariableNodes& get_variables()const = 0;
        R(IScope)
    };
}
