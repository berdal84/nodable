#pragma once

#include "fw/core/reflection/reflection"
#include "fw/core/Pool.h"

namespace ndbl
{
    // forward declaration
    class Node;
    using fw::pool::ID;

    /**
     * @class Base abstract class for any Node Component
     */
	class Component
	{
    public:
        Component();
        Component(Component&&) = default;
        Component& operator=(Component&&) = default;
        virtual ~Component() = default;
        ID<Node>         get_owner()const { return m_owner; }
        virtual void     set_owner(ID<Node> node);
	protected:
        ID<Node> m_owner;
		REFLECT_BASE_CLASS()
        POOL_REGISTRABLE(Component)
    };
}