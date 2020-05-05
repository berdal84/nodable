#pragma once

#include "Nodable.h"    // for constants and forward declarations
#include <string>

namespace Nodable{

	enum Type_{
		Type_Unknown,
		Type_Boolean,
		Type_Number,
		Type_String,
		Type_Value,
		Type_Entity,
		Type_Wire,
		Type_COUNT
	};

	/* 
		This class is variant implementation
	*/

	class Variant{
	public:
		Variant();
		~Variant();

		bool        isSet            ()const;	
		bool        isType           (Type_ _type)const;

		void        setValue         (const Variant*);
		void        setValue         (const std::string&);
		void        setValue         (const char*);
		void        setValue         (double);
		void        setValue         (bool);
		
		void        setType          (Type_ _type);

		Type_       getType          ()const;
		std::string getTypeAsString  ()const;

		bool        getValueAsBoolean()const;
		double      getValueAsNumber ()const;
		std::string getValueAsString ()const;

	private:
		void*       		data 				= NULL;
		Type_       		type 				= Type_Unknown;
	};
}