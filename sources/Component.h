#pragma once

#include "Node.h"
#include "Nodable.h"


namespace Nodable{
	class Component : public Node
	{
	public:

		Component() {}

		~Component(){};
		void       setOwner(Node* _entity){ owner = _entity; }
		Node*      getOwner()const{return owner;}

	private:
		Node* owner = nullptr;
	};
}