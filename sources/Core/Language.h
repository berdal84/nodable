#pragma once

#include "Nodable.h"   // for constants and forward declarations
#include <map>
#include <functional>

namespace Nodable {

	/**

	The Operator class store the identifier and the precedence of an operator.
	
	Example: "+", "-" and "*" are an identifiers
	         0u, 1u and 2u are their respective precedence.
	*/
	class Operator {
	public:
		Operator(std::string _identifier,
			     unsigned short _precedence) :
			identifier(_identifier),
			precedence(_precedence) {}

		std::string identifier;
		unsigned short precedence;
	};

	/* This enum identifies each kind of word for a language */
	enum TokenType_
	{
		TokenType_String,
		TokenType_Number,
		TokenType_Symbol,
		TokenType_Operator,
		TokenType_Boolean,
		TokenType_Parenthesis,
		TokenType_Comma,
		TokenType_COUNT,
		TokenType_Unknown
	};

	struct FunctionArg {
		FunctionArg(TokenType_, std::string);
		TokenType_ type;
		std::string name;
	};

	
	typedef std::function<int(Member*, const std::vector<const Member*>&)> CallableFunction;

	class FunctionPrototype {
	public:
		FunctionPrototype(std::string _identifier, TokenType_ _type);

		void                           pushArg(TokenType_ _type, std::string _name = "");
		bool                           match(FunctionPrototype& _other);
		const std::string&             getIdentifier()const;
		const std::string              getSignature()const;
		const std::vector<FunctionArg> getArgs() const;
		const TokenType_               getType() const;

		CallableFunction call;

	private:
		std::string identifier;
		std::vector<FunctionArg> args;
		TokenType_ type;
	};

	/**
		The Language class defines a single Language (ex: C++, Python, etc...)
		Some static constants provide easy access to ready to use Languages.
	*/
	typedef std::function<std::string(FunctionPrototype, std::vector<const Member*>)> SerializeFunction;

	class Language {
	public:

		Language(std::string _name) :numbers(), letters(), brackets(), name(_name){};
		~Language() {};		
		void                                  addOperator(Operator);
		unsigned short                        getOperatorPrecedence(const std::string& _identifier)const;
		std::string                           getOperatorsAsString()const;
		const FunctionPrototype*              find(FunctionPrototype& prototype) const;
		void                                  addToAPI(FunctionPrototype prototype);
		bool                                  needsToBeEvaluatedFirst(std::string op, std::string nextOp)const;
		const std::vector<FunctionPrototype>& getAPI()const { return api; }	
		
		/**
		  * To generate the Nodable Language reference
		  * New language generators will be found here later...
		  * ex: static const Language& CPlusPlus();
		  */
		static const Language* Nodable();

	private:
		/* Some language reference constants*/
		static const Language* NODABLE;

	public:
		std::string numbers;
		std::string letters;
		std::map<std::string, TokenType_> keywords;
		SerializeFunction serializeFunction;
	private:
		std::string name;
		std::vector<char> brackets;
		std::map<std::string, Operator> operators;
		std::vector<FunctionPrototype> api;
	};

}
