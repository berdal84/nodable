#pragma once

#include "Nodable.h"
#include "Object.h"

#define COMPONENT_CONSTRUCTOR(ClassName)ClassName()\
	{\
		setMember("__class__", #ClassName);\
	}

namespace Nodable{
	class Component : public Object
	{
	public:

		COMPONENT_CONSTRUCTOR(Component)

		~Component(){};
		void         setOwner(Entity* _entity){ owner = _entity; }
		Entity*      getOwner()const{return owner;}
		virtual void update(){};
	private:
		Entity* owner = nullptr;
	};
}