#pragma once

#include <unordered_set>
#include <algorithm>
#include <memory>
#include "R_internal_Meta.h"
#include "R_internal_Typename.h"
#include "R_internal_Type.h"

namespace Nodable::R
{
    class Class;
    typedef std::shared_ptr<Class> Class_ptr;
    typedef std::shared_ptr<const Class> Class_ptr_const;

    /**
     * Meta class to describe a class and get its information at runtime.
     */
    class Class : public Type
    {
    public:
        Class(const char *_name) : Type(_name, "Class", Typename::Class) {}

        bool is_child_of(Class_ptr _possible_parent_class, bool _selfCheck = true) const;
        void add_parent(Class_ptr _parent);
        void add_child(Class_ptr _child);

        template<class T> inline bool is() const { return is_child_of(T::Get_class(), true); }
        template<class T> inline bool is_not() const { return !is_child_of(T::Get_class(), true); }
    private:
        std::unordered_set<Class_ptr> m_parents;
        std::unordered_set<Class_ptr> m_children;
    };
}
