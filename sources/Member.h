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
		Visibility_AlwaysVisible                = 0,
		Visibility_VisibleOnlyWhenUncollapsed   = 1,
		Visibility_AlwaysHidden                 = 2,
		Visibility_Default                      = Visibility_AlwaysVisible
	};

	class Member{
	public:
		Member();
		~Member();

		bool                allows                   (Connection_)                   const;
		bool                isSet                    ()                                   const;	
		bool                isType                   (Type_ _type)                        const;

		/*
			Setters
		*/

		void                setConnectionFlags       (Connection_);
		void                setSourceExpression      (const char* _val);
		void                setInput                 (Member* _val);
		void  		        setName                  (const char* _name);
		void                setOwner                 (Object* _owner);

		void                setValue                 (const Member*);
		void                setValue                 (std::string);
		void                setValue                 (const char*);
		void                setValue                 (double);
		void                setValue                 (bool);

		void                setType                  (Type_ _type);
		void                setVisibility            (Visibility_ _v);

		/*
			Getters
		*/

		Object*             getOwner()                                                    const;
		Member*             getInput()                                                    const;
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
		Member*             input               = nullptr;
		std::string         sourceExpression    = "";
		std::string 		name 				= "Unknown";
		Variant       		data;
		Visibility_ 		visibility 			= Visibility_Default;
		Connection_ 	    connection 	        = Connection_Default;
	};
}