#pragma once
#include "SlotFlag.h"
#include "fw/core/Pool.h"

namespace ndbl
{
    // Forward declarations
    using fw::PoolID;
    using fw::ID8;
    class Node;
    class Slot;

    class SlotRef
    {
    public:
        static const SlotRef null;

        PoolID<Node> node;       // 32-bits
        ID8<Slot>    id;         //  8-bits
        SlotFlags    flags;      //  8-bits
        i16_t        unused;     // 16-bits extra

        SlotRef();
        SlotRef(const SlotRef&);
        SlotRef(SlotRef&&);
        SlotRef(const Slot&);
        SlotRef& operator=(const SlotRef& other);
        SlotRef& operator=(SlotRef&& other);
        bool operator!=(const SlotRef & other) const;
        bool operator==(const SlotRef & other) const;
        Slot* get() const;
        Slot* operator->() const;
    };

    static_assert(sizeof(SlotRef) == 8, "SlotRef should not be larger than 64-bits");

} // namespace ndbl