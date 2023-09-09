#pragma once
#include <vector>

#include "fw/core/reflection/reflection"
#include "fw/core/types.h"
#include "fw/core/Pool.h"

namespace ndbl
{
    // forward declarations
    class InstructionNode;
    class VariableNode;
    class Node;
    using fw::pool::PoolID;

    /**
     * @class Interface for a scope node
     */
    class IScope
    {
    public:
        using VariableNodeVec = std::vector<PoolID<VariableNode>>;
        virtual void                 get_last_instructions_rec(std::vector<InstructionNode *> &out) = 0;  // Get all the possible last instructions of this scope.
        virtual void                 add_variable(PoolID<VariableNode>) = 0;                              // Add a variable to this scope.
        virtual bool                 has_no_variable()const = 0;                                          // Check if scope is empty
        virtual void                 remove_variable(VariableNode *) = 0;                           // Remove a given variable fom this scope.
        virtual size_t               remove_all_variables() = 0;                                          // Remove all variables from this scope.
        virtual PoolID<VariableNode> find_variable(const std::string &_name) = 0;                         // Find a variable by name (identifier).
        virtual const VariableNodeVec& variables()const = 0;                                          // Get all the variables of this scope.
    };
}
