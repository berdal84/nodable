#pragma once
#include <string>
#include "Node_Slot.h"

namespace ndbl
{
    struct Node_Slot_Link
    {
        Node_Slot* tail;
        Node_Slot* head;

        Node_Slot_Link(): tail(nullptr), head(nullptr) {};
        Node_Slot_Link(Node_Slot* tail, Node_Slot* head);
        Node_Slot_Link(const Node_Slot_Link&) = default;

        Node_Slot_Link& operator=(const Node_Slot_Link& other) { tail = other.tail; head = other.head; return *this;}
                      operator bool () const { return tail != nullptr && head != nullptr; }
        bool          operator!=( const Node_Slot_Link& other ) const { return !(*this == other); }
        bool          operator==( const Node_Slot_Link &other ) const { return tail == other.tail && head == other.head; }
        Node_Slot_Flags     type() const { return tail->type(); /* both tail and head share the same type */ }
    };

    std::string to_string(const Node_Slot_Link& _slot);
}

