#pragma once
#include "tools/core/reflection/reflection"
#include "tools/core/Signals.h"
#include <string>

namespace ndbl
{
    // forward declared to avoid a dependency with Node.h
    class Node;

    class NodeComponent
    {
    public:
        DECLARE_REFLECT_virtual
        SIGNAL(on_reset_owner);

        NodeComponent();
        virtual     ~NodeComponent() {}

        void        reset_name(std::string name);
        const char* name() const { return m_name.c_str(); }
        Node*       node() { return owner(); } // alias for owner()
        const Node* node() const { return owner(); } // alias for owner()
        bool        has_owner() const { return m_owner != nullptr; }
        Node*       owner() const     { return m_owner; }
        void        reset_owner(Node* = nullptr);
    protected:
        std::string m_name;
        Node*       m_owner;
    };
}