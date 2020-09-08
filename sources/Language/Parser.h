#pragma once
#include <string>
#include <vector>

#include "Nodable.h"    // forward declarations
#include "Language.h"
#include "Token.h"

namespace Nodable{


	/*
		The role of this class is to convert code string to a Nodable graph.

		ex: "a+b" will became an Add node connected to two variable nodes a and b.
	*/
	class Parser
	{
	public:
		Parser(const Language* _language, Container* _container);
		~Parser();

		/* Override from Node class */
		bool eval(const std::string& );
	private:

		Member* tokenToMember(const Token& _token);

		Member* parseRootExpression();

		Member* parseFunctionCall(size_t& _tokenId);

		/* Parse a sub expression, a sub expression is like: "( expression )" */
		Member* parseParenthesisExpression(size_t& _tokenId);

		/* To parse three tokens (ex: "1+59", "98*4", "true and false", etc...)*/
		Member* parseBinaryOperationExpression(size_t& _tokenId, unsigned short _precedence = 0u, Member* _left = nullptr);

		/** To parse two tokens (ex: !true, -5, etc..) */
		Member* parseUnaryOperationExpression(size_t& _tokenId, unsigned short _precedence = 0u);

		/** To parse a primary expression (ex: "myVariable", "10.4", etc... ) */
		Member* parseAtomicExpression(size_t& _tokenId);

		/* Build a graph resursively starting at the token _tokenIndex reading up to _tokenIdMax tokens.*/
		Member* parseExpression(size_t& _tokenIndex, unsigned short _precedence = 0u, Member* _left = nullptr);

		/* Cut the member "expression" into tokens to identifies its type (cf. TokenType enum) */
		bool    tokenizeExpressionString(const std::string& _expression);

		/* Check if the existing tokens match with the syntax of the language. tokenize() should be called first */
		bool           isSyntaxValid	   ();

		/** Generate a string with all tokens with _tokens[_highlight] colored in green*/
		std::string logTokens(const std::vector<Token> _tokens, const size_t _highlight);

		/* Creates a new token given a _type, _string and _chanIndex and add it to the tokens.*/
		void           addToken			   (TokenType _type, std::string _string, size_t _charIndex);

		/* To store the result of the tokenizeExpressionString() method
		   contain a vector of Tokens to be converted to a Nodable graph by all parseXXX functions */
		std::vector<Token> tokens;

		/* The language that defines a dictionnary, some functions and operators*/
		const Language* language;

		/* The target container of the parser in which all generated nodes will be pushed into*/
		Container* container;
	};

}