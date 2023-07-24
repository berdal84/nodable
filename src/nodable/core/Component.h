#pragma once

#include "fw/core/reflection/reflection"

namespace ndbl
{
    // forward declaration
    class Node;

    /**
     * @class Base abstract class for any Node Component
     */
	class Component
	{
    public:
        Component(): m_owner(nullptr) {}
        Component (const Component&) = delete;                   // disable to avoid mistakes
        Component& operator= (const Component&) = delete;        //       ... same ...
        virtual ~Component() = default;
        Node*        get_owner()const { return m_owner; }
        virtual void set_owner(Node* node){ m_owner = node; }
	protected:
        Node* m_owner;
		REFLECT_BASE_CLASS()
    };
}