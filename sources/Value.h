#pragma once

#include "Nodable.h"    // for constants and forward declarations
#include "Variant.h"
#include <string>

namespace Nodable{

	enum ConnectionFlags_
	{
		ConnectionFlags_None 			= 0,
		ConnectionFlags_InputOnly 		= 1 << 1,
		ConnectionFlags_OutputOnly 		= 1 << 2,
		ConnectionFlags_InputAndOutput 	= ConnectionFlags_InputOnly | ConnectionFlags_OutputOnly,
		ConnectionFlags_Default			= ConnectionFlags_None
	};
	
	enum Visibility_{
		Visibility_Public    = 0,
		Visibility_Protected = 1,
		Visibility_Private   = 2,
		Visibility_Default   = Visibility_Public
	};

	class Member{
	public:
		Member();
		~Member();

		bool        isSet            ()const;	
		bool        isType           (Type_ _type)const;

		void        setConnectionFlags(ConnectionFlags_);
		void        setSourceExpression(const char* _val){sourceExpression = _val;}
		void        setInput         (Member* _val);
		void  		setName			 (const char* _name){name = _name;}
		void        setOwner         (Object* _owner){owner = _owner;}
		void        setValue         (const Member*);
		void        setValue         (std::string);
		void        setValue         (const char*);
		void        setValue         (double);
		void        setValue         (bool);
		
		void        setType          (Type_ _type){data.setType(_type);}
		void        setVisibility    (Visibility_ _v){visibility = _v;}

		bool        allows           (ConnectionFlags_)const;
		Object*		getOwner		 ()const{return owner;}
		Member*      getInput         ()const{return input;}
		const std::string& getName   ()const{return name;}
		std::string getSourceExpression()const;
		Type_       getType          ()const;
		std::string getTypeAsString  ()const;

		bool        getValueAsBoolean()const;
		double      getValueAsNumber ()const;
		std::string getValueAsString ()const;

		Visibility_ getVisibility    ()const{ return visibility;}
	private:
		Object*     		owner       		= nullptr;
		Member*              input               = nullptr;
		std::string         sourceExpression    = "";
		std::string 		name 				= "Unknown";
		Variant       		data;
		Visibility_ 		visibility 			= Visibility_Default;
		ConnectionFlags_ 	connectionFlags 	= ConnectionFlags_Default;
	};
}