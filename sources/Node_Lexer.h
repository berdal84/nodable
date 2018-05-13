#pragma once
#include "Nodable.h"    // forward declarations
#include "Node.h"       // base class
#include <string>
#include <vector>
#include <utility>      // for std::pair

namespace Nodable{
	typedef std::pair<std::string, std::string> Token; // TODO convert this to conventionnal Node_Variable

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
		void           addToken			   (std::string _category, std::string _string);
		std::vector<Token> tokens;
	};
}