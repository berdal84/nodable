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
    class NodeComponent : public TComponent<Node*>
	{
    public:
        POOL_REGISTRABLE(NodeComponent)
        REFLECT_DERIVED_CLASS()
    };

    template<class T>
    struct IsNodeComponent
    {
        using type = std::is_base_of<NodeComponent, T>;
        static constexpr bool value = IsNodeComponent<T>::type::value;
    };
}