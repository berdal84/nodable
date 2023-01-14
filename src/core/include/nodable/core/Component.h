#pragma once

#include <nodable/core/reflection/reflection>

namespace ndbl{

    class Node;

    /**
     * @class Base abstract class for any Node Component
     */
	class Component
	{
        friend Node;
    public:

        virtual bool    update() = 0;                             // Called each frame
        inline Node*    get_owner()const { return m_owner; }      // Get the node owning this component
        template<class T> [[nodiscard]]                           // Shorthand to cast<T>(this)
            inline T*       as() { return cast<T>(this); }
        template<class T> [[nodiscard]]                           // Shorthand to cast<const T>(this)
            inline const T* as()const { return cast<const T>(this); }

	protected:
		Component() = default;
		virtual ~Component() = default;
        Component (const Component&) = delete;
        Component& operator= (const Component&) = delete;
        /** Set the node owning this component */
		virtual void  set_owner(Node* node){ m_owner = node; }

	private:
		Node* m_owner = nullptr;
		REFLECT_BASE_CLASS()
    };
}