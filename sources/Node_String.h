#pragma once

#include "Nodable.h"
#include "Node_Value.h" // base class
#include <string>

namespace Nodable{
	/*
		The Node_String class node can store a string (internally as an std::string)
	*/
	class Node_String : public Node_Value{
	public:
		Node_String(const char* _value="");
		~Node_String();
		void        setValue(const char* /*value*/);
		const char* getValue()const;
		bool operator!=(const Node_String* _right){return value != _right->value;}
	private:
		std::string value;
	};
}
