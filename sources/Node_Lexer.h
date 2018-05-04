#pragma once
#include "Nodable.h"    // forward declarations
#include "Node.h"       // base class
#include <string>
#include <vector>
#include <utility>      // for std::pair


namespace Nodable{
	typedef std::pair<std::string, std::string> Token;

	class Node_Lexer : public Node
	{
	public:
		Node_Lexer(Node_String* _expression);
		virtual ~Node_Lexer();
		void           evaluate			                  ();
	private:
		void           buildExecutionTreeAndEvaluateRec   (size_t _tokenIndex, Node_Value* _finalRes, Node_Value* _prevRes = nullptr);
		void           tokenize			                  ();
		bool           isSyntaxValid		              ();
		void           buildExecutionTreeAndEvaluate      ();
		Node_Value*    convertTokenToNode                 (Token token);
		void           addToken			                  (std::string _category, std::string _string);

		Node_String*       expression;
		std::vector<Token> tokens;
	};
}