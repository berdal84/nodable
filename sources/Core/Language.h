#pragma once

#include "Nodable.h"   // for constants and forward declarations
#include <map>

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
		TokenType_COUNT,
		TokenType_Unknown
	};

	class FunctionPrototype {
	public:
		FunctionPrototype(std::string _identifier);
		void pushArgument(TokenType_ _type);
		bool match(FunctionPrototype& _other);
		const std::string& getIdentifier()const;
	private:
		std::string identifier;
		std::vector<TokenType_> arguments;
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

		/* Some language reference constants*/
		static const Language* NODABLE;

	private:

		/* To generate the Nodable Language reference */
		static const Language* Nodable();

		/* New language generators will be found here later... */
		/* ex: static const Language& CPlusPlus(); */

	public:
		std::string numbers;
		std::string letters;
		std::map<std::string, TokenType_> keywords;
	private:
		std::vector<char> brackets;
		std::map<std::string, Operator> operators;
	};

}
