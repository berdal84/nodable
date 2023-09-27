#pragma once

#include "SlotRef.h"
namespace ndbl
{
    class DirectedEdge
    {
    public:
        static DirectedEdge null;

        SlotRef tail;
        SlotRef head;

        DirectedEdge();
        DirectedEdge(const DirectedEdge & other);
        DirectedEdge(SlotRef _tail, SlotRef _head);

        DirectedEdge& operator=(const DirectedEdge& other);
        operator bool () const;
        bool operator==(const DirectedEdge& other) const;
        bool operator!=(const DirectedEdge& other) const;
    };

    std::string to_string(const DirectedEdge& edge);
}