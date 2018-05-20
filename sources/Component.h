#pragma once

#include "Nodable.h"
#include "Object.h"

namespace Nodable{
	class Component : public Object
	{
	public:

		Component()
		{
			addMember("componentType");
			setMember("componentType", "Unknown");
		};

		~Component(){};
		virtual void update(){};
	};
}