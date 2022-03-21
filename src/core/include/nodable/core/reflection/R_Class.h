#pragma once

#include <unordered_set>
#include <algorithm>
#include <memory>
#include "R_Type.h"
#include "R_MetaType.h"

namespace Nodable { namespace R
{
    class Class;
    typedef std::shared_ptr<Class> Class_ptr;
    typedef std::shared_ptr<const Class> Class_ptr_const;

    /**
     * Meta class to describe a class and get its information at runtime.
     */
    class Class : public MetaType
    {
    public:
        explicit Class(const char *_name) : MetaType(_name, Type::Class) {}

        bool is_child_of(Class_ptr _possible_parent_class, bool _selfCheck = true) const;
        void add_parent(Class_ptr _parent);
        void add_child(Class_ptr _child);
        template<class T> inline bool is_child_of() const { return is_child_of(T::Get_class(), true); }
        template<class T> inline bool is_not_child_of() const { return !is_child_of(T::Get_class(), true); }
    private:
        std::unordered_set<Class_ptr> m_parents;
        std::unordered_set<Class_ptr> m_children;
    };
} }
