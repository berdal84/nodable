#pragma once

#include <vector>

#include <nodable/core/reflection/R.h>
#include <nodable/core/Node.h>

namespace Nodable
{
    // forward declarations
    class Scope;
    class Member;
    class InstructionNode;

    /**
     * @brief Interface for any conditional structure node (ex: if/else, for, while, do/while )
     */
    class IConditionalStruct
    {
    public:
        virtual Member* condition_member()const = 0;
        virtual Scope* get_condition_true_scope()const = 0;
        virtual Scope* get_condition_false_scope()const = 0;
        virtual void   set_cond_instr(InstructionNode*) = 0;
        virtual InstructionNode* get_cond_instr()const = 0;
        R(IConditionalStruct)
    };
}