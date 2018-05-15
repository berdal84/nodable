#pragma once
#include "Nodable.h"    // forward declarations
#include "Node.h"       // base class
#include <string>
#include <vector>
#include <utility>      // for std::pair

namespace Nodable{
	/* This enum identifies each kind of Token the lexer can handle */
	enum TokenType_
	{
		TokenType_String   = 0,
		TokenType_Number   = 1,
		TokenType_Symbol   = 2,
		TokenType_Operator = 3,
		TokenType_Unknown  = 4
	};

	typedef struct
	{
		TokenType_  type      = TokenType_Unknown; // the type of the token
		std::string word      = "";                // the word as a string
		size_t      charIndex = 0;                 // the index of the first character of the token in the evaluated expression.
	}Token;

	class Node_Lexer : public Node
	{
	public:
		Node_Lexer();
		virtual ~Node_Lexer();
		bool           eval			       ()override;
	private:
		void           buildGraphRec       (size_t _tokenIndex, Node_Variable* _finalRes, Node_Variable* _prevRes = nullptr);
		void           tokenize			   ();
		bool           isSyntaxValid	   ();
		Node_Variable* buildGraph          ();
		Node_Variable* convertTokenToNode  (Token token);
		void           addToken			   (TokenType_ _type, std::string _string, size_t _charIndex);
		std::vector<Token> tokens;
	};
}