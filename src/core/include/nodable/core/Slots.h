#pragma once

#include <vector>
#include <observe/event.h>
#include <nodable/core/assertions.h>

namespace Nodable
{
    /**
     * @brief Slots is a vector-like container with a limited size and an event system.
     */
    template<typename T>
    struct Slots
    {
        Slots(T _parent, size_t _max_count = std::numeric_limits<size_t>::max())
            : m_parent(_parent)
            , m_limit(_max_count) {}

        std::vector<T>&       content() { return m_slots; }
        const std::vector<T>& content() const { return m_slots; }
        auto                  begin() const { return m_slots.begin(); }
        auto                  end() const { return m_slots.end(); }
        void                  set_limit(size_t _count) { m_limit = _count; }
        size_t                get_limit()const { return m_limit; }
        T                     back() { return m_slots.back(); }
        bool                  empty() const { return m_slots.empty(); }
        size_t                size() const { return m_slots.size(); }
        T                     operator[](size_t _index) const { return m_slots[_index]; }
        bool                  accepts() const { return m_slots.size() < m_limit; }

        void remove(T _node)
        {
            auto found = std::find(m_slots.begin(), m_slots.end(), _node);
            m_slots.erase(found);
            m_on_removed.emit(_node);
        }

        void add(T _node)
        {
            NODABLE_ASSERT(m_slots.size() < m_limit);
            m_slots.push_back(_node);
            m_on_added.emit(_node);
        }

        T get_front_or_nullptr() const
        {
            return m_slots.empty() ? nullptr : m_slots[0];
        }

        T get_back_or_nullptr() const
        {
            return m_slots.empty() ? nullptr : m_slots.back();
        }

        observe::Event <T> m_on_added;
        observe::Event <T> m_on_removed;

    private:
        T               m_parent;
        size_t          m_limit;
        std::vector <T> m_slots;
    };
}