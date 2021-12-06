#pragma once

#include <vector>

#include <nodable/Node.h>

namespace Nodable
{
    // forward declarations
    class ScopedCodeBlockNode;
    class Member;
    class InstructionNode;

    /**
     * @brief Interface for any Contitional structure node (ex: if/else, for, while, do/while )
     */
    class AbstractConditionalStruct
    {
    public:
        /**
         * Set a condition to be evaluated in order to branch to the right scope.
         */
        virtual void                   set_condition(Member*) const = 0;
        virtual Member*                get_condition()const = 0;
        /**
         * Get the branch to follow if condition is evaluated true
         */
        virtual ScopedCodeBlockNode*   get_condition_true_branch()const = 0;
        /**
         * Get the branch to follow if condition is evaluated false
         */
        virtual ScopedCodeBlockNode*   get_condition_false_branch()const = 0;
    };
}