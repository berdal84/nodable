#pragma once

#include "Nodable.h"    // for constants and forward declarations
#include "Variant.h"
#include <string>

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

		bool                allows                   (Connection_)                   const;
		bool                isSet                    ()                              const;	
		bool                isType                   (Type_)                         const;

		/*
			Setters
		*/

		void                setConnectionFlags       (Connection_);
		void                setSourceExpression      (const char*);
		void                setInputMember           (Member*);
		void  		        setName                  (const char*);
		void                setOwner                 (Object*);

		void                setValue                 (const Member*);
		void                setValue                 (std::string);
		void                setValue                 (const char*);
		void                setValue                 (double);
		void                setValue                 (bool);

		void                setType                  (Type_ _type);
		void                setVisibility            (Visibility_ _v);

		/** Get the value of the inputMember Member and set it to this Member.
		    Warning: be sure the member has an inputMember before calling this (getInputMember()!=nullptr)*/
		void                updateValueFromInputMemberValue();

		/*
			Getters
		*/

		Object*             getOwner()                                                    const;
		Member*             getInputMember()                                              const;
		const std::string&  getName()                                                     const;
		std::string         getSourceExpression()                                         const;
		Type_               getType()                                                     const;
		std::string         getTypeAsString()                                             const;

		bool                getValueAsBoolean()                                           const;
		double              getValueAsNumber()                                            const;
		std::string         getValueAsString()                                            const;

		Visibility_         getVisibility()                                               const;
		Connection_         getConnection()                                               const;

	private:
		Object*     		owner       		= nullptr;
		Member*             inputMember         = nullptr;
		std::string         sourceExpression    = "";
		std::string 		name 				= "Unknown";
		Variant       		data;
		Visibility_ 		visibility 			= Default;
		Connection_ 	    connection 	        = Connection_Default;
	};
}