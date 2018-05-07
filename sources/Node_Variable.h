#pragma once

#include "Nodable.h"    // forward declarations and common stuff
#include "Node_Value.h" // base class
#include <string>

namespace Nodable{
	/* Node_Variable is a node that identify a value with its name */
	class Node_Variable : public Node_Value{
	public:
		Node_Variable(const char* _name, Node* _target = nullptr);
		~Node_Variable();

		void            updateLabel();

		void            setName         (const char*);
		void            setValue        (Node* _node)override;
		void            setValue        (const char* /*value*/)override;
		void            setValue        (double /*value*/)override;
		
		const char*     getName()const;
		Node* 	        getValueAsNode  ()override;
		double          getValueAsNumber()const override;
		std::string     getValueAsString()const override;
	private:
		Node*       target;
		std::string name;
	};
}