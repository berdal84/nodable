#pragma once
#include <string>
#include <vector>
#include <stack>

#include <nodable/Nodable.h> // forward declarations
#include <nodable/Language.h>
#include <nodable/Token.h>
#include <nodable/TokenRibbon.h>

namespace Nodable{

    // forward declaration
    class InstructionNode;
    class ScopedCodeBlockNode;
    class AbstractCodeBlock;
    class CodeBlockNode;

	/**
		The role of this class is to convert code string to a Nodable graph.

		The main strategy is:
		- cut string into tokens
		- parse token list to convert it to Nodable graph.

		ex: "a+b" will became an Add node connected to two VariableNode* a and b.

	    There is no AST (Abstract Syntax Tree) since Nodable keep graph (Nodes) linked to text (tokens) all the time.
	*/

	class Parser
	{
	public:
		explicit Parser(const Language* _language): language(_language), graph(nullptr){}
		virtual ~Parser(){}

		/** Evaluates an expression as a string.
		   Return true if evaluation went well and false otherwise. */
		bool expressionToGraph(const std::string &_code, GraphNode* _graphNode );

    protected:

	    /** Start a parsing transaction.
	     * This will memorise the current cursor position on the token ribbon.
	     * A commit/rollback must follow. */
        void startTransaction();
        void rollbackTransaction();
        void commitTransaction();

        bool parseBool(const std::string& _str);
        std::string parseString(const std::string& _str);
        double parseDouble(const std::string& _str);

        /**
		 * Convert a Token to a Member.
	     * @return a Member* that owns _token.
	     */
		Member* tokenToMember(Token* _token);

        ScopedCodeBlockNode* parseScope();

		/** Parse a single instruction */
        InstructionNode* parseInstruction();

        Member* parseVariableDecl();

        CodeBlockNode* parseCodeBlock();

        ConditionalStructNode* parseConditionalStructure();

        CodeBlockNode* parseProgram();

		/** Parse a Function call starting at current cursor position.
		   Return the result as a Member or nullptr if parsing failed. */
		Member* parseFunctionCall();

		/** Parse a sub expression starting at current cursor position.
		   A sub expression is like: "( expression )>
		   Return the result as Member* or nullptr if parsing failed. */
		Member* parseParenthesisExpression();

		/** Parse binary operation expression starting at a specific token index.
		   _precedence is the precedence value of the previous operator.
		   _left is the left handed side of the operation.
		*/
		Member* parseBinaryOperationExpression(unsigned short _precedence = 0u, Member* _left = nullptr);

		/** To parse two tokens (ex: !true, -5, etc..) */
		Member* parseUnaryOperationExpression(unsigned short _precedence = 0u);

		/** To parse a primary expression (ex: "myVariable", "10.4", etc... ) */
		Member* parseAtomicExpression();

		/** Build a graph recursively starting at the current cursor position. */
		Member* parseExpression(unsigned short _precedence = 0u, Member* _left = nullptr);

		/** Split a given expression string into tokens (cf. Token) */
		bool tokenizeExpressionString(const std::string& _expression);

		/** Check if the token ribbon match with the language's syntax.
		 * tokenizeExpressionString() must be called first */
		bool isSyntaxValid();

		/** Get the current scope (during parsing, scope changes, we need to know the current to push any new variable) */
        ScopedCodeBlockNode* getCurrentScope();

        /** Given a Literal token, return its type */
        Type getTokenLiteralType(const Token* _token )const;

		/** A language to get Semantic and Syntax (not yet implemented) */
		const Language* language;

		/** The target container of the parser in which all generated nodes will be pushed into*/
		GraphNode* graph;

		/** A token ribbon to store a token list and be able to eat them easily */
		TokenRibbon tokenRibbon;

    };

}