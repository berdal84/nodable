#pragma once

#include <string>

#include <Nodable.h>    // forward declarations and common stuff
#include <Node.h>       // base class
#include <Member.h>
#include "mirror_macros.h"

namespace Nodable{
	/* Variable is a node that identify a value with its name */
	class Variable : public Node {
	public:
		Variable();
		~Variable();

		void              setName         (const char*);
		bool              isSet           ()const{return getMember()->isSet(); }
		bool              isType          (Type _type)const;		
		const char*       getName         ()const;

		template <typename T>
		T as()const {
			return  get("value")->get<T>();
		}

		Member* getMember()const {
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