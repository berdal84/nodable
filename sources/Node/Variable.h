#pragma once

#include <string>

#include <Nodable.h>    // forward declarations and common stuff
#include <Node.h>       // base class
#include <Member.h>

namespace Nodable{
	
	/*
		The role of this class is to wrap a variable as a Node.

		The variable can be accessed through a single Member called "value".
		The value member can be linked to other node members.
	*/
	class Variable : public Node {
	public:
		Variable();
		~Variable();

		void              setName         (const char*);
		bool              isSet           ()const{return value()->isDefined(); }
		bool              isType          (Type _type)const;		
		const char*       getName         ()const;

		Member* value()const {
			return get("value");
		}

		std::string       getTypeAsString ()const;
	private:
		std::string       name;
	public:

		template<class Value>
		void set(Value _value)
		{
			get("value")->set(_value);
			updateLabel();
		};

		template<class Value>
		void set(Value* _value)
		{
			get("value")->set(_value);
			updateLabel();
		};

		MIRROR_CLASS(Variable)(
			MIRROR_PARENT(Node)
		);
    };
}