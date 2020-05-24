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

	class Variant {
	public:
		Variant();
		~Variant();

		bool        isSet()const;
		bool        isType(Type_ _type)const;
		void        set(const Variant*);
		void        set(const std::string&);
		void        set(const char*);
		void        set(double);
		void        set(bool);
		void        setType(Type_ _type);
		Type_       getType()const;
		std::string getTypeAsString()const;

		operator double()const;
		operator bool()const;
		operator std::string()const;

	private:
		void* data = NULL;
		Type_ type = Type_Unknown;
	};
}