#pragma once

#include <nodable/Nodable.h>
#include <nodable/Reflect.h>

namespace Nodable{

    class Node;
    
	class Component
	{
	public:

		Component() = default;
		virtual ~Component() = default;

		virtual bool update() = 0;
		virtual inline void  setOwner(Node* _entity){ owner = _entity; }
		[[nodiscard]] inline Node* getOwner()const { return owner; }

		template<class T> [[nodiscard]] T* as()
		{
		    if( getClass()->isChildOf(T::GetClass()))
                return reinterpret_cast<T*>(this);
            return nullptr;
		}

        template<class T> [[nodiscard]] const T* as()const
        {
            if( getClass()->isChildOf(T::GetClass()))
                return reinterpret_cast<const T*>(this);
            return nullptr;
        }

	private:
		Node* owner = nullptr;
		REFLECT(Component)
    };
}