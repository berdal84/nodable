#pragma once

#include "tools/core/reflection/reflection"

namespace ndbl
{
    // forward declarations
    class Node;
    class Scope;
    class Slot;
    class Property;
    class InstructionNode;

    typedef size_t Branch;
    enum Branch_ : size_t
    {
        Branch_FALSE   = 0,
        Branch_TRUE    = 1,
    };

    /**
     * @interface Interface for any conditional structure node (ex: if/else, for, while, do/while )
     */
    class IConditional
    {
    public:
        virtual Scope*         get_scope_at(Branch) const = 0;
        virtual Slot&          get_child_slot_at(Branch) = 0;
        virtual const Slot&    get_child_slot_at(Branch) const = 0;
        virtual Slot&          get_condition_slot(Branch) = 0;
        virtual const Slot&    get_condition_slot(Branch) const = 0;
        virtual Node*          get_condition(Branch) const = 0;
        REFLECT_BASE_CLASS()
    };
}