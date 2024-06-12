#pragma once

#include "tools/core/reflection/reflection"
#include "tools/core/memory/memory.h"
#include "TComponent.h"

namespace ndbl
{
    // forward declaration
    class Node;

    /**
     * @class Base abstract class for any Node Component
     */
    class NodeComponent : public TComponent< ::tools::PoolID<Node> >
	{
    public:
        POOL_REGISTRABLE(NodeComponent)
        REFLECT_DERIVED_CLASS()
    };
}