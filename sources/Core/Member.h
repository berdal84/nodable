#pragma once

#include "Nodable.h"    // for constants and forward declarations
#include "Variant.h"
#include "Language.h"
#include "Connector.h"
#include "Visibility.h"

#include <string>

namespace Nodable{

	class Member{
	public:
		Member();
		~Member();

		bool                allows(Way)const;
		bool                isSet()const;	
		bool                isType(Type)const;
		bool                equals(const Member *)const;
		void                setConnectorWay(Way);
		void                setSourceExpression(const char*);
		void                setInputMember(Member*);
		void  		        setName(const char*);
		void                setOwner(Object*);	
		void                set(const Member*);
		void                set(const Member&);
		void                set(double);
		void                set(int);
		void                set(const std::string&);
		void                set(const char* _value);
		void                set(bool _value);
		void                setType(Type _type);
		void                setVisibility(Visibility _v);
		void                unset() { data = Variant(); }

		/** Get the value of the inputMember Member and set it to this Member.
		    Warning: be sure the member has an inputMember before calling this (getInputMember()!=nullptr)*/
		void                updateValueFromInputMemberValue();

		Object*             getOwner()const;
		Member*             getInputMember()const;
		const std::string&  getName()const;
		std::string         getSourceExpression()const;
		Type                getType()const;
		std::string         getTypeAsString()const;

		inline operator bool()const        { return data; }
		inline operator double()const      { return data; }
		inline operator std::string()const { return data; }

		Visibility          getVisibility()const;
		Way                 getConnectorWay()const;		
		const Connector*    input() const;
		const Connector*    output() const;

	private:
		Object*     		owner       		= nullptr;
		Member*             inputMember         = nullptr;
		std::string         sourceExpression    = "";
		std::string 		name 				= "Unknown";
		Variant       		data;
		Visibility 		    visibility 			= Visibility::Default;
		Connector*          in                  = nullptr;
		Connector*          out                 = nullptr;

	};
}