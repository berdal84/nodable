#pragma once
#include "Component.h"
#include "tools/core/reflection/Type.h"

namespace ndbl
{
    // forward declared to avoid a dependency with Node.h
    class Node;

    BASE_COMPONENT_CLASS_EX(Node, NodeComponent)
    (
    public:
        Node*       node()       { return owner(); } // alias for owner()
        const Node* node() const { return owner(); } // alias for owner()

        REFLECT_BASE_CLASS()
    );
}