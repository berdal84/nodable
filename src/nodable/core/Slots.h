#pragma once
#include <observe/event.h>
#include <vector>
#include "fw/core/assertions.h"
#include "core/DirectedEdge.h"

namespace ndbl
{
    enum class SlotEvent
    {
        ADD,
        REMOVE,
    };

    constexpr size_t Slots_LIMIT_MAX = 64 /* std::numeric_limits<size_t>::max() */;

    /**
     * @struct Vector-like container with a limited size and an event system. Element are stored as T
     */
    template<typename T>
    struct Slots
    {
        static_assert(sizeof(T) <= sizeof(size_t), "T should not be larger than a size_t. Use T* or ID<T> instead.");

        Slots(Edge_t edge_type, size_t limit)
            : m_limit( limit )
            , m_edge_type( edge_type )
        {}

        Slots(Slots&& other)
        { *this = std::move(other); }

        Slots<T>& operator=(Slots<T>&& other)
        {
            if( this == &other ) return *this;

            m_limit     = other.m_limit;
            m_edge_type = other.m_edge_type;
            m_slots     = std::move( other.m_slots );
            on_add      = std::move( other.on_add );
            on_remove   = std::move( other.on_remove );
            on_change   = std::move( other.on_change );
            return *this;
        }

        std::vector<T>&       content() { return m_slots; }       // Get slots content as a vector
        const std::vector<T>& content() const { return m_slots; } // Get slots content as a vector
        auto      begin() const { return m_slots.begin(); }       // Get an iterator at the beginning of the slots
        auto      end() const { return m_slots.end(); }           // Get an iterator at the end of the slots
        void      set_limit(size_t _count) { m_limit = _count; }  // Set a slot count limit
        size_t    get_limit()const { return m_limit; }            // Get the slot count limit
        T         back() { return m_slots.back(); }               // Get the last slot (check if not empty() before)
        bool      empty() const { return m_slots.empty(); }       // Check if slots are empty
        size_t    size() const { return m_slots.size(); }         // Get the slot count
        T         operator[](size_t _index) const { return m_slots[_index]; } // Get the element at zero-based index
        bool      accepts() const { return m_slots.size() < m_limit; }        // Check if accepts new elements (depends on limit)
        Edge_t    edge_type() const { return m_edge_type; }

        void      apply(SlotEvent event, T item, bool notify = true)
        {
            switch (event)
            {
                case SlotEvent::ADD:    return add( item, notify );
                case SlotEvent::REMOVE: return remove( item, notify );
            }
            FW_EXPECT(false, "Unhandled case");
        }

        void remove(T item, bool notify = true)
        {
            auto it = std::find(m_slots.begin(), m_slots.end(), item );
            FW_ASSERT( it != m_slots.end() );
            m_slots.erase( it );

            if ( notify )
            {
                on_remove.emit( item );
                on_change.emit( item, SlotEvent::REMOVE, m_edge_type );
            }
        }

        void add(T item, bool notify = true )
        {
            FW_ASSERT(m_slots.size() < m_limit);
            m_slots.push_back( item );

            if ( notify )
            {
                on_add.emit( item );
                on_change.emit( item, SlotEvent::ADD, m_edge_type );
            }
        }

        T first() const
        { return m_slots.empty() ? T{} : m_slots[0]; }

        T last() const
        { return m_slots.empty() ? T{} : m_slots.back(); }

        observe::Event<T, SlotEvent, Edge_t> on_change;
        observe::Event<T> on_add;    // Triggered when an item is added
        observe::Event<T> on_remove; // Triggered when an item is removed

    private:
        Edge_t         m_edge_type;
        size_t         m_limit;   // maximum item count
        std::vector<T> m_slots;   // item container
    };
}