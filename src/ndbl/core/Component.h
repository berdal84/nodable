#pragma once

#include "tools/core/reflection/reflection"
#include "tools/core/memory/memory.h"

namespace ndbl
{
    // forward declaration
    class Node;

    /**
     * @class Base abstract class for any Node Component
     */
	class Component
	{
        POOL_REGISTRABLE(Component)
        REFLECT_BASE_CLASS()
    public:
        Component();
        Component(Component&&) = default;
        Component& operator=(Component&&) = default;
        virtual ~Component() = default;
        ::tools::PoolID<Node> get_owner()const { return m_owner; }
        virtual void set_owner(::tools::PoolID<Node> node);
	protected:
        ::tools::PoolID<Node> m_owner;
    };
}