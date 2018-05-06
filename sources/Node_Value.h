#pragma once

#include "Nodable.h"    // for constants and forward declarations
#include "Node.h"       // the base class.
#include "vector"
#include <string>
#include "stdlib.h"		// for size_t

namespace Nodable{
	/* 
		The Node_Value class is the base class for basic types such as Numbers, Strings or Booleans
	*/
	class Node_Value : public Node{
	public:
		Node_Value(Type_ _type);
		virtual ~Node_Value();

		virtual void        setValue         (Node*)=0;
		virtual void        setValue         (const char* /*value*/)=0;
		virtual void        setValue         (double /*value*/)=0;

		virtual Node*       getValueAsNode   (){return this;}
		virtual double      getValueAsNumber ()const=0;
		virtual std::string getValueAsString ()const=0;		

		Type_               getType          ()const;
		bool                isType           (Type_ _type)const;		
	private:
		Type_ type;
	};
}