#pragma once

#include <fw/core/reflection/reflection>

namespace ndbl
{
    // forward declaration
    class Node;

    /**
     * @class Base abstract class for any Node Component
     */
	class Component
	{
        friend Node; // tightly coupled
    public:
        Component(): m_owner(nullptr) {}
        Component (const Component&) = delete;                    // disable to avoid mistakes
        Component& operator= (const Component&) = delete;         //       ... same ...
        virtual ~Component() = default;
        inline Node*    get_owner()const { return m_owner; }      // Get the node owning this component
        template<class T> [[nodiscard]]                           // Shorthand to cast<T>(this)
            inline T*       as() { return fw::cast<T>(this); }
        template<class T> [[nodiscard]]                           // Shorthand to cast<const T>(this)
            inline const T* as()const { return fw::cast<const T>(this); }

	protected:
        virtual void  set_owner(Node* node){ m_owner = node; }     // Set the component's owner

        Node* m_owner;                                             // The Node which own this component

		REFLECT_BASE_CLASS()
    };
}