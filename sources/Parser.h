#pragma once
#include "Nodable.h"    // forward declarations
#include "Entity.h"       // base class
#include "Language.h"
#include <string>
#include <vector>

namespace Nodable{

	/*

		This class is build to generate a graph from an expression string.
		You just have to instantiate it and set its member "expression" and call update();

		auto container = new Container("MyGlobalContainer");
		auto lexer     = container->createLexer( Language::NODABLE );
		lexer->setMember("expression", "10+50*40/0.001");
		lexer->update();	

		// here container should contain the generated graph.

	*/
	class Parser : public Entity
	{
	private:

		typedef struct
		{
			TokenType_  type      = TokenType_Unknown; // the type of the token
			std::string word      = "";                // the word as a string
			size_t      charIndex = 0;                 // the index of the first character of the token in the evaluated expression.

			bool isOperand()const {
				return type == TokenType_Number ||
					   type == TokenType_Boolean ||
					   type == TokenType_String ||
					   type == TokenType_Symbol;
			}

		}Token;

	public:
		Parser(const Language* _language);
		virtual ~Parser();

		/* Override from Entity class */
		bool           eval			       ();
	private:

		Member* operandTokenToMember(const Token& _token);

		Member* buildGraphIterative();

		Member* parseRootExpression();

		/* Parse a sub expression, a sub expression is like: "( expression )" */
		Member* parseParenthesisExpression(size_t& _tokenId, unsigned short _depth = 0u);

		/* To parse three tokens (ex: "1+59", "98*4", "true and false", etc...)*/
		Member* parseBinaryOperationExpression(size_t& _tokenId, unsigned short _precedence = 0u, Member* _left = nullptr, unsigned short _depth = 0u);

		/** To parse two tokens (ex: !true, -5, etc..) */
		Member* parseUnaryOperationExpression(size_t& _tokenId, unsigned short _precedence = 0u, unsigned short _depth = 0u);

		/** To parse a primary expression (ex: "myVariable", "10.4", etc... ) */
		Member* parseAtomicExpression(size_t& _tokenId, unsigned short _depth = 0u);

		/* Build a graph resursively starting at the token _tokenIndex reading up to _tokenIdMax tokens.*/
		Member* parseExpression(size_t& _tokenIndex, unsigned short _precedence = 0u, Member* _left = nullptr, unsigned short _depth = 0u);

		/* Cut the member "expression" into tokens to identifies its type (cf. TokenType_ enum) */
		bool    tokenizeExpressionString			   ();

		/* Check if the existing tokens match with the syntax of the language. tokenize() should be called first */
		bool           isSyntaxValid	   ();

		/** Compute a prefix string with n times (_tabCount) a specific string (_tabChars)*/
		static inline const std::string ComputePrefix(const unsigned short _tabCount, const std::string _tabChar = " ") {
			std::string result("");

			for (unsigned short i = 0u; i < _tabCount; i++) {
				result.append(_tabChar);
			}

			return result;
		}

		/** Generate a string with all tokens with hightlight token colored in green*/
		std::string Parser::LogTokens(const std::vector<Token> _tokens, const size_t _highlight);

		/* Creates a new token given a _type, _string and _chanIndex and add it to the tokens.*/
		void           addToken			   (TokenType_ _type, std::string _string, size_t _charIndex);

		std::vector<Token> tokens;
		const Language* language;
	};

}