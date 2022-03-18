#pragma once

#include <vector>

#include <nodable/core/reflection/R.h>
#include <nodable/core/Node.h>

namespace Nodable
{
    // forward declarations
    class Scope;
    class Member;

    /**
     * @brief Interface for any conditional structure node (ex: if/else, for, while, do/while )
     */
    class IConditionalStruct
    {
    public:
        virtual Member* condition_member()const = 0;
        virtual Scope* get_condition_true_branch()const = 0;
        virtual Scope* get_condition_false_branch()const = 0;

        R(IConditionalStruct)
    };
}