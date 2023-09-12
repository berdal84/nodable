#pragma once
#include "SlotFlag.h"
#include "fw/core/Pool.h"

namespace ndbl
{
    // Forward declarations
    using fw::PoolID;
    using fw::ID;
    class Node;
    class Slot;

    struct SlotRef
    {
        static const SlotRef null;

        ID<Slot>     id;
        PoolID<Node> node;
        SlotFlags    flags;
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
} // namespace ndbl