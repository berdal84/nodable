#pragma once

#include "Nodable.h"
#include "Object.h"

#define DECLARE_COMPONENT(ClassName)ClassName()\
	{\
		setMember("__class__", #ClassName);\
	}

namespace Nodable{
	class Component : public Object
	{
	public:

		DECLARE_COMPONENT(Component)

		~Component(){};
		void         setOwner(Entity* _entity){ owner = _entity; }
		Entity*      getOwner()const{return owner;}
		virtual void update(){};
	private:
		Entity* owner = nullptr;
	};
}