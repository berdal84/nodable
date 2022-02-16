#pragma once

#include <unordered_set>

namespace Nodable::R
{
    /**
     * Meta class to describe a class and get its information at runtime.
     */
    class Class
    {
    public:
        Class(const char *_name) : m_name(_name) {}

        bool is_child_of(const Class *_possible_parent_class, bool _selfCheck = true) const
        {
            bool is_child;

            if (_selfCheck && this == _possible_parent_class)
            {
                is_child = true;
            }
            else if ( m_parents.empty())
            {
                is_child = false;
            }
            else
            {
                auto direct_parent_found = std::find(m_parents.begin(), m_parents.end(), _possible_parent_class);

                // direct parent check
                if ( direct_parent_found != m_parents.end())
                {
                    is_child = true;
                }
                else // indirect parent check
                {
                    bool is_a_parent_is_child_of = false;
                    for (Class *each : m_parents)
                    {
                        if (each->is_child_of(_possible_parent_class, true))
                        {
                            is_a_parent_is_child_of = true;
                        }
                    }
                    is_child = is_a_parent_is_child_of;
                }
            }
            return is_child;
        };

        const char *get_name() const
        {
            return m_name;
        }

        void add_parent(Class *_parent)
        {
            m_parents.insert(_parent);
        }

        void add_child(Class *_child)
        {
            m_children.insert(_child);
        }

        template<class T>
        inline bool is() const { return is_child_of(T::Get_class(), true);  }

        template<class T>
        inline bool is_not() const { return !is_child_of(T::Get_class(), true); }

    private:
        const char *         m_name;
        std::unordered_set<Class *> m_parents;
        std::unordered_set<Class *> m_children;
    };

    template<typename T>
    struct is_reflected { static constexpr bool value = std::is_member_function_pointer<decltype(&T::get_class)>::value; };

    /**
     * Cast a _source instance from Src to Dst type.
     * If not possible returns nullptr
     *
     * @tparam Src the source type
     * @tparam Dst
     * @param _source
     * @return
     */
    template<class Dst, class Src>
    inline Dst* cast_pointer(Src *_source)
    {
        static_assert( is_reflected<Src>::value, "class Src is not reflected by R" );
        static_assert( is_reflected<Dst>::value, "class Dst is not reflected by R" );

        if(_source->get_class()->is_child_of(Dst::Get_class()))
            return dynamic_cast<Dst*>(_source);
        return nullptr;
    };
}
