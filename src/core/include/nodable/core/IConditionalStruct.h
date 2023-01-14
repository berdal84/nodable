#pragma once

#include <nodable/core/reflection/reflection>

namespace ndbl
{
    // forward declarations
    class Scope;
    class Member;
    class InstructionNode;

    /**
     * @interface Interface for any conditional structure node (ex: if/else, for, while, do/while )
     */
    class IConditionalStruct
    {
    public:
        /** Get the condition member (contains the value of the condition) */
        virtual Member* condition_member()const = 0;

        /** Get the the "true" branch's scope */
        virtual Scope* get_condition_true_scope()const = 0;

        /** Get the the "false" branch's scope */
        virtual Scope* get_condition_false_scope()const = 0;

        /** Set the condition expression to evaluate */
        virtual void set_cond_expr(InstructionNode*) = 0;

        /** Get the condition expression to evaluate */
        virtual InstructionNode*get_cond_expr()const = 0;
    };

}