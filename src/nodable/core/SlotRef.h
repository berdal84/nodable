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
        Slot* get() const;
        Slot* operator->() const;
    };
} // namespace ndbl