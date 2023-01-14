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
        /** Called each frame */
        virtual bool  update() = 0;

        /** Get the node owning this component */
        inline Node*  get_owner()const { return m_owner; }

        /** Shorthand to cast<T>(this) */
        template<class T> [[nodiscard]] inline       T* as()      { return cast<T>(this); }

        /** Shorthand to cast<const T>(this) */
        template<class T> [[nodiscard]] inline const T* as()const { return cast<const T>(this); }

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