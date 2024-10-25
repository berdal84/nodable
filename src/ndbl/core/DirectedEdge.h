#pragma once
#include <string>
#include "Slot.h"

namespace ndbl
{
    struct DirectedEdge
    {
        Slot* tail;
        Slot* head;

        DirectedEdge(): tail(nullptr), head(nullptr) {};
        DirectedEdge(Slot* tail, Slot* head);
        DirectedEdge(const DirectedEdge&) = default;

        inline DirectedEdge& operator=(const DirectedEdge& other) { tail = other.tail; head = other.head; return *this;}
        inline               operator bool () const { return tail != nullptr && head != nullptr; }
        inline bool          operator!=( const DirectedEdge& other ) const { return !(*this == other); }
        inline bool          operator==( const DirectedEdge &other ) const { return tail == other.tail && head == other.head; }
        inline SlotFlags     type() const { return tail->type(); /* both tail and head share the same type */ }
    };

    std::string to_string(const DirectedEdge& _slot);
}

