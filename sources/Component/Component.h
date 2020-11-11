#pragma once

#include "Object.h"

#include <mirror.h>

namespace Nodable{

    class Node;

	class Component: public Object
	{
	public:

		Component() {}

		~Component(){};
		virtual bool update() { return true; };
		virtual void setOwner(std::weak_ptr<Node> _node){ owner = std::move(_node); }
		std::shared_ptr<Node> getOwner()const{return owner.lock();}

	protected:
		std::weak_ptr<Node> owner;
		MIRROR_CLASS(Component)(
			MIRROR_PARENT(Object);
		);
	};
}