#pragma once

#include "Nodable.h"    // for constants and forward declarations
#include <string>

namespace Nodable{

	enum Type_{
		Type_Unknown,
		Type_Boolean,
		Type_Number,
		Type_String,
		Type_COUNT
	};
	
	/* 
		This class is variant implementation
	*/

	class Value{
	public:
		Value(Type_ _type = Type_Unknown);
		~Value();

		void        setValue         (const Value&);
		void        setValue         (std::string);
		void        setValue         (const char*);
		void        setValue         (double);
		void        setValue         (bool);

		bool        getValueAsBoolean()const;
		double      getValueAsNumber ()const;
		std::string getValueAsString ()const;		
		std::string getTypeAsString  ()const;

		Type_       getType          ()const;
		bool        isType           (Type_ _type)const;
		void        setType          (Type_ _type){type = _type;}
		bool        isSet            ()const;	

	private:
		void*       data = NULL;
		Type_       type = Type_Unknown;
	};
}