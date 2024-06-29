#pragma once

#include "ndbl/core/Token.h"
#include "tools/core/memory/memory.h"
#include "tools/core/reflection/variant.h"
#include "tools/core/types.h"// for constants and forward declarations

#include <string>
#include <vector>

namespace ndbl
{
    // forward declarations
    class Node;

    typedef int PropertyFlags;
    enum PropertyFlag_
    {
        PropertyFlag_NONE            = 0,
        PropertyFlag_IS_REF          = 1 << 0,
        PropertyFlag_IS_PRIVATE      = 1 << 1,
        PropertyFlag_IS_THIS         = 1 << 2, // Property pointing this Property's parent Node (stored as void* in variant).
        PropertyFlag_ALL             = ~PropertyFlag_NONE,
    };

    /**
     * @class The class store a value (as a variant) and is owned by a PropertyGroup
     *
     * A property is like a property in OOP, you can set its visibility, type, and value.
     * In Nodable, a property can also be connected (see DirectedEdge) to another property is they "way" allows it.
     */
	class Property
    {
    public:
        Token token;

        void               init(const tools::type*, PropertyFlags, Node*); // must be called once before use
        void               digest(Property *_property);
        bool               has_flags(PropertyFlags flags)const { return (m_flags & flags) == flags; };
        void               set_flags(PropertyFlags flags) { m_flags |= flags; }
        void               clear_flags(PropertyFlags flags = PropertyFlag_ALL) { m_flags &= ~flags; }
        void               set_name(const char* _name) { m_name = _name; }
        Node*              get_owner()const { return m_owner; }
        const std::string& get_name()const { return m_name; }
        const tools::type* get_type()const { return m_type; }
        bool               is_type(const tools::type* other) const;
    private:
        Node*              m_owner = nullptr;
        PropertyFlags      m_flags = PropertyFlag_NONE;
        const tools::type* m_type  = nullptr;
        std::string        m_name;
    };
}