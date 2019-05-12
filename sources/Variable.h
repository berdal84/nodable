#pragma once

#include "Nodable.h"    // forward declarations and common stuff
#include "Entity.h"       // base class
#include "Member.h"
#include <string>

namespace Nodable{
	/* Variable is a node that identify a value with its name */
	class Variable : public Entity{
	public:
		Variable();
		~Variable();

		void              setName         (const char*);

		template<typename T>
		void              setValue        (T _value);
		
		template<typename T>
		void              setValue        (T* _value);

		bool              isSet           ()const{return getValue()->isSet(); }
		bool              isType          (Type_ _type)const;		
		const char*       getName         ()const;
		double            getValueAsNumber()const;
		std::string       getValueAsString()const;
		Member*            getValue        ()const{return getMember("value");}
		std::string       getTypeAsString ()const;
	private:
		std::string       name;
	};


	template<class Value>
	void Variable::setValue(Value _value)
	{
		getMember("value")->setValue(_value);
		updateLabel();
	}

	template<class Value>
	void Variable::setValue(Value* _value)
	{
		getMember("value")->setValue(_value);
		updateLabel();
	}
}