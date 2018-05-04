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
		virtual ~Node_String();

		bool        isEqualsTo          (const Node_String* _right)const;
		bool        isEmpty             ()const;

		void        setValue            (Node*)override{};
		void        setValue            (const char* /*value*/)override;
		void        setValue            (double /*value*/)override;		

		double      getValueAsNumber    ()const;
		std::string getValueAsString    ()const;

		std::string getLabel            ()const;
	private:
		std::string value;
	};
}
