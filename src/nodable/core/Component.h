#pragma once

#include "fw/core/reflection/reflection"
#include "fw/core/memory/Pool.h"

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
        ::fw::PoolID<Node> get_owner()const { return m_owner; }
        virtual void set_owner(::fw::PoolID<Node> node);
	protected:
        ::fw::PoolID<Node> m_owner;
    };
}