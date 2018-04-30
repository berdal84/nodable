#pragma once

#include "Nodable.h"
#include "Node_Value.h" // base class
#include <string>

namespace Nodable{
	/*
		The class Node_Number is a node that can memorize a number (typed as a double)
	*/
	class Node_Number : public Node_Value{
	public:
		~Node_Number();
		Node_Number(double _n);
		Node_Number(std::string _string);
		virtual void   draw           ()override;
		double getValue()const;
		void   setValue(double _value);
	private:
		double value;
	};
}