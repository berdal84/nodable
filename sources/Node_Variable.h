#pragma once

#include "Nodable.h"    // forward declarations and common stuff
#include "Node.h"       // base class
#include "Node_Value.h"
#include <string>

namespace Nodable{
	/* Node_Variable is a node that identify a value with its name */
	class Node_Variable : public Node{
	public:
		Node_Variable();
		~Node_Variable();

		void    updateLabel()override;

		void            setName         (const char*);

		template<typename T>
		void setValue(T _value);

		
		bool            isSet           ()const{return getValue().isSet(); }
		bool            isType          (Type_ _type)const;
		
		const char*       getName()const;
		double            getValueAsNumber()const;
		std::string       getValueAsString()const;
		const Node_Value& getValue()const{return getMember("value");}
		std::string       getTypeAsString()const;
	private:
		std::string       name;
	};


	template<class Node_Value>
	void Node_Variable::setValue(Node_Value _value)
	{
		setMember("value", _value);
		updateLabel();
	}
}