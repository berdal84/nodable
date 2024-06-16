#pragma once
#include <string>

namespace ndbl
{
    // Forward declarations
    class Slot;

    class DirectedEdge
    {
    public:
        static DirectedEdge null;

        Slot* tail;
        Slot* head;

        DirectedEdge() = default;
        DirectedEdge(const DirectedEdge & other) = default;
        DirectedEdge(Slot* _tail, Slot* _head);

        DirectedEdge& operator=(const DirectedEdge& other);
        operator bool () const;
        bool operator==(const DirectedEdge& other) const;
        bool operator!=(const DirectedEdge& other) const;
    };

    std::string to_string(const DirectedEdge& _slot);
}