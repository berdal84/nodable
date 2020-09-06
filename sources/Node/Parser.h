#pragma once
#include <string>
#include <vector>

#include "Nodable.h"    // forward declarations
#include "Node.h"       // base class
#include "Language.h"
#include "Token.h"

namespace Nodable{

	/*

		This class is build to generate a graph from an expression string.
		You just have to instantiate it and set its member "expression" and call update();

		auto container = new Container("MyGlobalContainer");
		auto lexer     = container->createLexer( Language::NODABLE );
		lexer->set("expression", "10+50*40/0.001");
		lexer->update();	

		// here container should contain the generated graph.

	*/
	class Parser : public Node
	{
	public:
		Parser(const Language* _language);
		virtual ~Parser();

		/* Override from Node class */
		bool           eval			       ();
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
		bool    tokenizeExpressionString			   ();

		/* Check if the existing tokens match with the syntax of the language. tokenize() should be called first */
		bool           isSyntaxValid	   ();

		/** Generate a string with all tokens with _tokens[_highlight] colored in green*/
		std::string logTokens(const std::vector<Token> _tokens, const size_t _highlight);

		/* Creates a new token given a _type, _string and _chanIndex and add it to the tokens.*/
		void           addToken			   (TokenType _type, std::string _string, size_t _charIndex);

		std::vector<Token> tokens;
		const Language* language;
	};

}