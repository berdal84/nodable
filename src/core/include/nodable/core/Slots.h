#pragma once

#include <vector>
#include <observe/event.h>
#include <nodable/core/assertions.h>

namespace ndbl
{
    /**
     * @struct Vector-like container with a limited size and an event system.
     */
    template<typename T>
    struct Slots
    {
        Slots() = delete;
        Slots(T _parent, size_t _max_count = std::numeric_limits<size_t>::max())
            : m_parent(_parent)
            , m_limit(_max_count) {}

        /** Get slots as a vector */
        std::vector<T>&       content() { return m_slots; }
        /** Get slots as a vector */
        const std::vector<T>& content() const { return m_slots; }
        /** Get an iterator at the beginning of the slots */
        auto                  begin() const { return m_slots.begin(); }
        /** Get an iterator at the end of the slots */
        auto                  end() const { return m_slots.end(); }
        /** Set a slot count limit */
        void                  set_limit(size_t _count) { m_limit = _count; }
        /** Get the slot count limit */
        size_t                get_limit()const { return m_limit; }
        /** Get the last slot (check if not empty() before) */
        T                     back() { return m_slots.back(); }
        /** Check if slots are empty */
        bool                  empty() const { return m_slots.empty(); }
        /** Get the slot count */
        size_t                size() const { return m_slots.size(); }
        /** Get the element at zero-based index */
        T                     operator[](size_t _index) const { return m_slots[_index]; }
        /** Check if accepts new elements (depends on limit) */
        bool                  accepts() const { return m_slots.size() < m_limit; }

        /** Remove a given node */
        void remove(T _node)
        {
            auto found = std::find(m_slots.begin(), m_slots.end(), _node);
            m_slots.erase(found);
            m_on_removed.emit(_node);
        }

        /** Add a given node */
        void add(T _node)
        {
            NDBL_ASSERT(m_slots.size() < m_limit);
            m_slots.push_back(_node);
            m_on_added.emit(_node);
        }

        /** Get a pointer to the first slot or nullptr if empty */
        T get_front_or_nullptr() const
        {
            return m_slots.empty() ? nullptr : m_slots[0];
        }

        /** Get a pointer to the last slot or nullptr if empty */
        T get_back_or_nullptr() const
        {
            return m_slots.empty() ? nullptr : m_slots.back();
        }

        /** Triggered when a node is added */
        observe::Event <T> m_on_added;
        /** Triggered when a node is removed */
        observe::Event <T> m_on_removed;

    private:
        T               m_parent;
        size_t          m_limit;
        std::vector <T> m_slots;
    };
}