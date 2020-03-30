#pragma once

#include "Entity.h"
#include "Nodable.h"


namespace Nodable{
	class Component : public Entity
	{
	public:

		Component() {}

		~Component(){};
		void         setOwner(Entity* _entity){ owner = _entity; }
		Entity*      getOwner()const{return owner;}

	private:
		Entity* owner = nullptr;
	};
}