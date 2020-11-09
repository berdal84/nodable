#pragma once
#include <string>
#include <utility>
#include <vector>

#include "Language.h"
#include "Token.h"

namespace Nodable{

    class Container;

	/*
		The role of this class is to convert code string to a Nodable graph.

		The main strategy is:
		- cut string into tokens
		(TODO: before converting to Nodable graph, we need to build an AST)
		- parse token list to convert it to Nodable graph.

		ex: "a+b" will became an Add node connected to two Variable* a and b.

		*:Variable is a Node extended class
	*/

	class Parser
	{
	public:
		/* The Parser need at least:
		   - a Language (to understand the code)
		   - a Container (to store the result)
		*/
        Parser::Parser( std::shared_ptr<const Language> _language,
                        std::shared_ptr<Container> _container ):
                language(std::move(_language)),
                container(std::move(_container)){}
		~Parser() = default;

		/* Evaluates an expression as a string.
		   Return true if evaluation went well and false otherwise. */
		bool eval(const std::string& );

	private:

        /* Parse the root expression.
           The root expression is set when calling eval().
           Return the result as a Member or nullptr if parsing failed. */
        std::shared_ptr<Member> parseRootExpression();

		/* Parse a Function call starting at a specific token index.
		   Return the result as a Member or nullptr if parsing failed. */
        std::shared_ptr<Member> parseFunctionCall(size_t& _tokenId);

		/* Parse a sub expression starting at a specific token index.
		   A sub expression is like: "( expression )"
		   Return the result as Member or nullptr if parsing failed. */
        std::shared_ptr<Member> parseParenthesisExpression(size_t& _tokenId);

		/* Parse binary operation expression starting at a specific token index.
		   _precedence is the precedence value of the previous operator.
		   _left is the left handed side of the operation.
		*/
        std::shared_ptr<Member> parseBinaryOperationExpression(size_t& _tokenId, unsigned short _precedence = 0u, std::shared_ptr<Member> _left = std::shared_ptr<Member>(nullptr));

		/** To parse two tokens (ex: !true, -5, etc..) */
        std::shared_ptr<Member> parseUnaryOperationExpression(size_t& _tokenId, unsigned short _precedence = 0u);

		/** To parse a primary expression (ex: "myVariable", "10.4", etc... ) */
        std::shared_ptr<Member> parseAtomicExpression(size_t& _tokenId);

		/* Build a graph resursively starting at the token _tokenIndex reading up to _tokenIdMax tokens.*/
        std::shared_ptr<Member> parseExpression(size_t& _tokenIndex, unsigned short _precedence = 0u, std::shared_ptr<Member> _left = std::shared_ptr<Member>(nullptr));

		/* Cut the member "expression" into tokens to identifies its type (cf. TokenType enum) */
		bool tokenizeExpressionString(const std::string& _expression);

		/* Check if the existing tokens match with the syntax of the language. tokenize() should be called first */
		bool isSyntaxValid();

		/** Generate a string with all tokens with _tokens[_highlight] colored in green*/
		static std::string LogTokensWithHighlightedAtIndex(const std::vector<Token>& _tokens, size_t _highlightedTokenIndex);

		/* Adds a new token given a _type, _string and _charIndex and add it to the tokens.*/
		void addToken(const TokenType _type, const std::string& _string, size_t _charIndex);

		/* To store the result of the tokenizeExpressionString() method
		   contain a vector of Tokens to be converted to a Nodable graph by all parseXXX functions */
		std::vector<Token> tokens;

		/* The language that defines a dictionnary, some functions and operators*/
		std::shared_ptr<const Language> language;

		/* The target container of the parser in which all generated nodes will be pushed into*/
		std::shared_ptr<Container> container;
	};

}