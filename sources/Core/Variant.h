#pragma once

#include "Nodable.h"    // for constants and forward declarations
#include "Type.h"
#include <string>
#include <any>

namespace Nodable{

	/* 
		This class is variant implementation
	*/

	class Variant {
	public:
		Variant();
		~Variant();

		bool        isSet()const;
		bool        isType(Type _type)const;
		void        set(const Variant*);
		void        set(const std::string&);
		void        set(const char*);
		void        set(double);
		void        set(bool);
		void        setType(Type _type);
		Type       getType()const;
		std::string getTypeAsString()const;

		operator double()const;
		operator bool()const;
		operator std::string()const;

	private:
	    std::any data;
		Type type = Type::Any;
	};
}