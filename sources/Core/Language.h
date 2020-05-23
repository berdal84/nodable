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

	class FunctionPrototype {
	public:
		FunctionPrototype(std::string _identifier, TokenType_ _type);

		void                           pushArg(TokenType_ _type);
		bool                           match(FunctionPrototype& _other);
		const std::string&             getIdentifier()const;
		const std::vector<FunctionArg> getArgs() const;
		const TokenType_               getType() const;
		std::function<int(Member*, const std::vector<const Member*>&)> nativeFunction;

	private:
		std::string identifier;
		std::vector<FunctionArg> args;
		TokenType_ type;
	};

	/**
		The Language class defines a single Language (ex: C++, Python, etc...)
		Some static constants provide easy access to ready to use Languages.
	*/
	class Language {
	public:

		Language() :numbers(), letters(), brackets() {};
		~Language() {};		
		void addOperator(Operator);
		unsigned short getOperatorPrecedence(const std::string& _identifier)const;
		std::string getOperatorsAsString()const;
		const FunctionPrototype* findFunctionPrototype(FunctionPrototype& prototype) const;
		void pushFunc(FunctionPrototype prototype);
		bool needsToBeEvaluatedFirst(std::string op, std::string nextOp)const;

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
	private:
		std::vector<char> brackets;
		std::map<std::string, Operator> operators;
		std::vector<FunctionPrototype> functionPrototypes;
	};

}
