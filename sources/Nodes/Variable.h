#pragma once

#include <string>

#include <Nodable.h>    // forward declarations and common stuff
#include <Node.h>       // base class
#include <Member.h>

namespace Nodable{
	/* Variable is a node that identify a value with its name */
	class Variable : public Node {
	public:
		Variable();
		~Variable();

		void              setName         (const char*);
		bool              isSet           ()const{return getValue()->isSet(); }
		bool              isType          (Type_ _type)const;		
		const char*       getName         ()const;
		double            getValueAsNumber()const;
		std::string       getValueAsString()const;
		Member*           getValue        ()const{return get("value");}
		std::string       getTypeAsString ()const;

	private:
		std::string       name;

	public:
		template<class Value>
		void setValue(Value _value)
		{
			get("value")->setValue(_value);
			updateLabel();
		};

		template<class Value>
		void setValue(Value* _value)
		{
			get("value")->setValue(_value);
			updateLabel();
		};
	};
}