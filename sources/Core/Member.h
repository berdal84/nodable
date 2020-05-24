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

	class Connector {
	public:

		Connector(Member* _member = nullptr,
			      Connection_ _side = Connection_Default):
			member(_member),
			side(_side){
		};


		bool equals(const Connector* _other)const {
			return this->member == _other->member &&
				this->side == _other->side;
		};

		~Connector() {};

		Member* member;
		Connection_ side;

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
		void                set(const Member*);
		void                set(double);
		void                set(int);
		void                set(const std::string&);
		void                set(const char* _value);
		void                set(bool _value);
		void                setType(Type_ _type);
		void                setVisibility(Visibility_ _v);
		void                unset() { data = Variant(); }

		/** Get the value of the inputMember Member and set it to this Member.
		    Warning: be sure the member has an inputMember before calling this (getInputMember()!=nullptr)*/
		void                updateValueFromInputMemberValue();

		Object*             getOwner()const;
		Member*             getInputMember()const;
		const std::string&  getName()const;
		std::string         getSourceExpression()const;
		Type_               getType()const;
		std::string         getTypeAsString()const;

		inline operator bool()const        { return data; }
		inline operator double()const      { return data; }
		inline operator std::string()const { return data; }

		Visibility_         getVisibility()const;
		Connection_         getConnectionFlags()const;		
		const Connector*    input() const;
		const Connector*    output() const;

	private:
		Object*     		owner       		= nullptr;
		Member*             inputMember         = nullptr;
		std::string         sourceExpression    = "";
		std::string 		name 				= "Unknown";
		Variant       		data;
		Visibility_ 		visibility 			= Default;
		Connector*          in                  = nullptr;
		Connector*          out                 = nullptr;
	public:
		static const TokenType_ MemberTypeToTokenType(Type_ _type);
		static const Type_      TokenTypeToMemberType(TokenType_ _tokenType);		
	};
}