#pragma once

#include "Nodable.h"    // for constants and forward declarations
#include "Variant.h"
#include <string>
#include "Language.h"

namespace Nodable{

	enum Connection_
	{
		Connection_None     = 0,
		Connection_In       = 1 << 1,
		Connection_Out      = 1 << 2,
		Connection_InOut 	= Connection_In | Connection_Out,
		Connection_Default	= Connection_None
	};
	
	enum Visibility_{
		Always                = 0,
		OnlyWhenUncollapsed   = 1,
		Hidden                = 2,
		Default               = Always
	};

	class Member{
	public:
		Member();
		~Member();

		bool                allows(Connection_)const;
		bool                isSet()const;	
		bool                isType(Type_)const;
		bool                equals(const Member *)const;
		void                setConnectionFlags(Connection_);
		void                setSourceExpression(const char*);
		void                setInputMember(Member*);
		void  		        setName(const char*);
		void                setOwner(Object*);
		
		template<typename T>
		void setValue(T _value) { data.set(_value); }

		template<>
		void setValue<>(Member* _value){
			data.set(&_value->data);	
		}

		void                setType(Type_ _type);
		void                setVisibility(Visibility_ _v);

		/** Get the value of the inputMember Member and set it to this Member.
		    Warning: be sure the member has an inputMember before calling this (getInputMember()!=nullptr)*/
		void                updateValueFromInputMemberValue();

		Object*             getOwner()const;
		Member*             getInputMember()const;
		const std::string&  getName()const;
		std::string         getSourceExpression()const;
		Type_               getType()const;
		std::string         getTypeAsString()const;
		bool                getValueAsBoolean()const;

		template <typename T>
		operator T()const {
			return this->data;
		}

		std::string         getValueAsString()const;
		Visibility_         getVisibility()const;
		Connection_         getConnection()const;		
	private:
		Object*     		owner       		= nullptr;
		Member*             inputMember         = nullptr;
		std::string         sourceExpression    = "";
		std::string 		name 				= "Unknown";
		Variant       		data;
		Visibility_ 		visibility 			= Default;
		Connection_ 	    connection 	        = Connection_Default;

	public:
		static const TokenType_ MemberTypeToTokenType(Type_ _type);
		static const Type_      TokenTypeToMemberType(TokenType_ _tokenType);	
	};

	
}