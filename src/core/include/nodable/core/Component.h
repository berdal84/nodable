#pragma once

#include <nodable/core/types.h>
#include <nodable/core/reflection/R.h>

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
		template<class T> [[nodiscard]] inline       T* as()      { return R::cast_class_ptr<T>(this); }
        template<class T> [[nodiscard]] inline const T* as()const { return R::cast_class_ptr<const T>(this); }
	private:
		Node* m_owner = nullptr;
		R(Component)
    };
}