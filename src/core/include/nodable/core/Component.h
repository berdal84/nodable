#pragma once

#include <nodable/core/types.h>
#include <nodable/core/reflection/reflection>

namespace Nodable{

    class Node;
    
	class Component
	{
	public:

		Component() = default;
		virtual ~Component() = default;
        Component (const Component&) = delete;
        Component& operator= (const Component&) = delete;

		virtual bool         update() = 0;
		virtual inline void  set_owner(Node *_entity){ m_owner = _entity; }
		inline Node*         get_owner()const { return m_owner; }
		template<class T> [[nodiscard]] inline       T* as()      { return cast<T>(this); }
        template<class T> [[nodiscard]] inline const T* as()const { return cast<const T>(this); }
	private:
		Node* m_owner = nullptr;
		REFLECT_BASE_CLASS()
    };
}