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
		}Token;

	public:
		Parser(const Language* _language);
		virtual ~Parser();

		/* Override from Entity class */
		bool           eval			       ();
	private:

		Member* operandTokenToMember(const Token& _token);

		Member* buildGraphIterative();
		/* To parse more than three tokens (ex: "1+59+1", "98*4*1", "true and false or true", etc...)*/
		Member* parseBinaryOperationExpressionEx(size_t& _tokenId, unsigned short _precedence, Member* _leftOverride, Member* _rightOverride);

		/* To parse three tokens (ex: "1+59", "98*4", "true and false", etc...)*/
		Member* parseBinaryOperationExpression(size_t& _tokenId, unsigned short _precedence, Member* _leftOverride, Member* _rightOverride);

		/** To parse two tokens (ex: !true, -5, etc..) */
		Member* parseUnaryOperationExpression(size_t& _tokenId, unsigned short _precedence );

		/** To parse a primary expression (ex: "myVariable", "10.4", etc... ) */
		Member* parsePrimaryExpression(size_t& _tokenId);

		/* Build a graph resursively starting at the token _tokenIndex reading up to _tokenIdMax tokens.*/
		Member* parseExpression(size_t& _tokenIndex, unsigned short _precedence = 0u, Member* _leftValueOverride = nullptr, Member* _rightValueOverride = nullptr);

		/* Cut the member "expression" into tokens to identifies its type (cf. TokenType_ enum) */
		void           tokenizeExpressionString			   ();

		/* Check if the existing tokens match with the syntax of the language. tokenize() should be called first */
		bool           isSyntaxValid	   ();
		
		/* Creates a new token given a _type, _string and _chanIndex and add it to the tokens.*/
		void           addToken			   (TokenType_ _type, std::string _string, size_t _charIndex);

		std::vector<Token> tokens;
		const Language* language;
	};

}