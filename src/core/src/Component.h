#pragma once

#include "Nodable.h"
#include <mirror.h>

namespace Nodable::core{

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
		    if( this->getClass()->isChildOf(mirror::GetClass<T>()))
                return reinterpret_cast<T*>(this);
            return nullptr;
		}

        template<class T> [[nodiscard]] const T* as()const
        {
            if( this->getClass()->isChildOf(mirror::GetClass<T>()))
                return reinterpret_cast<const T*>(this);
            return nullptr;
        }

	private:
		Node* owner = nullptr;
		MIRROR_CLASS(Component)();
    };
}