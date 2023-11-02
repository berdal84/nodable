#pragma once

#include "fw/core/reflection/reflection"

namespace ndbl
{
    // forward declarations
    class Scope;
    class Slot;
    class Property;
    class InstructionNode;
    using fw::PoolID;

    typedef size_t Branch;
    enum Branch_ : size_t
    {
        Branch_FALSE = 0,
        Branch_TRUE = 1,
    };

    /**
     * @interface Interface for any conditional structure node (ex: if/else, for, while, do/while )
     */
    class IConditional
    {
    public:
        virtual PoolID<Scope>  get_scope_at(size_t _branch) const = 0;
        virtual Slot&          get_child_slot_at(size_t _branch) = 0;
        virtual const Slot&    get_child_slot_at(size_t _branch) const = 0;
        REFLECT_BASE_CLASS()
    };
}