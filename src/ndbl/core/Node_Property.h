#pragma once

#include "tools/core/reflection/Type_Descriptor.h"
#include "ndbl/core/Token.h"
#include <string>

namespace ndbl
{
    // forward declarations
    class Node;

    typedef int Node_Property_Flags;
    enum Node_Property_Flag_
    {
        Node_Property_Flag_NONE            = 0,
        Node_Property_Flag_IS_REF          = 1 << 0,
        Node_Property_Flag_IS_NODE_VALUE   = 1 << 1,
        Node_Property_Flag_ALL             = ~Node_Property_Flag_NONE,
    };

    // Property wraps a Token including extra inFormation such as: name, owner (Node), and some flags.
	class Node_Property
    {
    public:
        void                init(Node* owner, const tools::Type_Descriptor*, Node_Property_Flags, const char* _name); // must be called once before use
        void                digest(Node_Property *_property);
        bool                has_flags(Node_Property_Flags flags)const { return (m_flags & flags) == flags; };
        void                set_flags(Node_Property_Flags flags) { m_flags |= flags; }
        void                clear_flags(Node_Property_Flags flags = Node_Property_Flag_ALL) { m_flags &= ~flags; }
        //void              set_name(const char* _name) { m_name = _name; } names are indexed in PropertyBag, can't change
        Node_Property_Flags flags()const { return m_flags; }
        const std::string&  name()const { return m_name; }
        Node*               node()const { return m_node; }
        const tools::Type_Descriptor* get_type() const { return m_type; }
        bool                is_type(const tools::Type_Descriptor* other) const;
        void                set_type(const tools::Type_Descriptor *pDescriptor);
        void                set_token(const Token& _token) { m_token = _token; }
        Token&              token() { return m_token; }
        const Token&        token() const { return m_token; }

    private:
        Token               m_token;
        Node*               m_node  = nullptr; // owner
        Node_Property_Flags m_flags = Node_Property_Flag_NONE;
        std::string         m_name;
        const tools::Type_Descriptor* m_type = nullptr;
    };
}