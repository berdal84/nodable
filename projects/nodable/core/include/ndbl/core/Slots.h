#pragma once
#include <observe/event.h>
#include <vector>
#include <fw/core/assertions.h>

namespace ndbl
{
    /**
     * @struct Vector-like container with a limited size and an event system.
     */
    template<typename T>
    struct Slots
    {
        Slots(size_t _max_count = std::numeric_limits<size_t>::max())
            : m_limit(_max_count) {}

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

        void remove(T item)                                                   // Remove a given item from the slots
        {
            auto found = std::find(m_slots.begin(), m_slots.end(), item);
            m_slots.erase(found);
            m_on_removed.emit(item);
        }

        void add(T item)                                                     // Add a given item to the slots
        {
            FW_ASSERT(m_slots.size() < m_limit);
            m_slots.push_back(item);
            m_on_added.emit(item);
        }

        T get_front_or_nullptr() const                                        // Get a pointer to the first item or nullptr if empty
        {
            return m_slots.empty() ? nullptr : m_slots[0];
        }

        T get_back_or_nullptr() const                                         // Get a pointer to the last item or nullptr if empty
        {
            return m_slots.empty() ? nullptr : m_slots.back();
        }

        observe::Event <T> m_on_added;                                       // Triggered when an item is added
        observe::Event <T> m_on_removed;                                     // Triggered when an item is removed

    private:
        size_t          m_limit;                                             // maximum item count
        std::vector <T> m_slots;                                             // item container
    };
}