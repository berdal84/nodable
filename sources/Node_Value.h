#pragma once

#include "Nodable.h"    // for constants and forward declarations
#include <string>

namespace Nodable{

	/* 
		The Node_Value class is the base class for basic types such as Numbers, Strings or Booleans
	*/
	class Node_Value{
	public:
		Node_Value();
		~Node_Value();

		void        setValue         (const Node_Value&);
		void        setValue         (std::string);
		void        setValue         (const char*);
		void        setValue         (double);
		double      getValueAsNumber ()const;
		std::string getValueAsString ()const;		
		std::string getTypeAsString  ()const;

		Type_       getType          ()const;
		bool        isType           (Type_ _type)const;
		void        setType          (Type_ _type){type = _type;}
		bool        isSet            ()const;	

	private:
		std::string s = "";
		double      d = 0.0F;
		Type_   type = Type_Unknown;
	};
}