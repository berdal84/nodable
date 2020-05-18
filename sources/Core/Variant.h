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

		bool        isSet()const;	
		bool        isType(Type_ _type)const;
		void        set(const Variant*);
		void        set(const std::string&);
		void        set(const char*);
		void        set(double);
		void        set(int);
		void        set(bool);		
		void        setType(Type_ _type);
		Type_       getType()const;
		std::string getTypeAsString  ()const;
		bool        getValueAsBoolean()const;

		operator double()const{
			switch(type){
				case Type_String:  return double((*reinterpret_cast<std::string*>(data)).size());
				case Type_Number:  return *reinterpret_cast<double*>(data);
				case Type_Boolean: return *reinterpret_cast<bool*>(data) ? 1.0 : 0.0;
				default:           return 0.0;
			}			
		}

		operator int()const{
			return int( (double)*this );
		}

		std::string getValueAsString ()const;

	private:
		void* data = NULL;
		Type_ type = Type_Unknown;
	};
}