#pragma once

#include "Entity.h"
#include "Nodable.h"

#define COMPONENT_CONSTRUCTOR(ClassName)ClassName()\
	{\
		setMember("__class__", #ClassName);\
	}

namespace Nodable{
	class Component : public Entity
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