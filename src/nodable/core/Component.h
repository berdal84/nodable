#pragma once

#include "fw/core/reflection/reflection"
#include "fw/core/Pool.h"

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
        PoolID<Node> get_owner()const { return m_owner; }
        virtual void set_owner(PoolID<Node> node);
	protected:
        PoolID<Node> m_owner;
    };
}