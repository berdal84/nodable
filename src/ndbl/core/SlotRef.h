#pragma once
#include "SlotFlag.h"
#include "tools/core/memory/Pool.h"

namespace ndbl
{
    // Forward declarations
    using tools::PoolID;
    using tools::ID8;
    class Node;
    class Slot;

    class SlotRef
    {
    public:
        static const SlotRef null;

        PoolID<Node> node;
        ID8<Slot>    id;
        SlotFlags    flags;

        SlotRef();
        SlotRef(const SlotRef&);
        SlotRef(SlotRef&&) noexcept;
        SlotRef(const Slot&);
        SlotRef& operator=(const SlotRef& other);
        SlotRef& operator=(SlotRef&& other);
        Slot& operator * () { return *get(); }
        const Slot& operator * () const { return *get(); }
        explicit operator bool () const { return node && id; }
        bool operator!=(const SlotRef & other) const;
        bool operator==(const SlotRef & other) const;
        SlotFlags slot_type() const;
        Slot* get() const;
        Slot* operator->() const;
    };
} // namespace ndbl