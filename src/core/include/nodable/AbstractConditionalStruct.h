#pragma once

#include <vector>

#include <nodable/Reflect.h>
#include <nodable/Node.h>

namespace Nodable
{
    // forward declarations
    class Scope;
    class Member;
    class InstructionNode;

    /**
     * @brief Interface for any Contitional structure node (ex: if/else, for, while, do/while )
     */
    class AbstractConditionalStruct
    {
    public:
        /**
         * Get a condition to be evaluated in order to branch to the right scope.
         */
        virtual Member*                get_condition()const = 0;
        /**
         * Get the branch to follow if condition is evaluated true
         */
        virtual Scope*        get_condition_true_branch()const = 0;
        /**
         * Get the branch to follow if condition is evaluated false
         */
        virtual Scope*         get_condition_false_branch()const = 0;

        REFLECT(AbstractConditionalStruct)
    };
}