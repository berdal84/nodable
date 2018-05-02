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
		Node_Number(double _n);
		Node_Number(const char*);
		virtual ~Node_Number();
		virtual void   draw             ()override;

		void        setValue            (Node*)override{};
		void        setValue            (double)override;
		void        setValue            (const char*)override;

		double      getValueAsNumber    ()const;
		std::string getValueAsString    ()const;

		std::string getLabel            ()const;
	private:
		double value;
	};
}