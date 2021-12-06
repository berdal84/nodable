#pragma once
#include <string>
#include <vector>
#include <stack>

#include <nodable/Nodable.h> // forward declarations
#include <nodable/Language.h>
#include <nodable/Token.h>
#include <nodable/TokenRibbon.h>
#include "ForLoopNode.h"

namespace Nodable{

    // forward declaration
    class InstructionNode;
    class ScopedCodeBlockNode;
    class AbstractCodeBlock;
    class CodeBlockNode;
    class ForLoopNode;

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
		explicit Parser(const Language* _language)
            : m_language(_language)
            , m_graph(nullptr){}
		~Parser(){}

		/** Evaluates an expression as a string.
		   Return true if evaluation went well and false otherwise. */
		bool                   expression_to_graph(const std::string &_code, GraphNode *_graphNode);

    protected:

	    /** Start a parsing transaction.
	     * This will memorise the current cursor position on the token ribbon.
	     * A commit/rollback must follow. */
        void                   start_transaction();
        void                   rollback_transaction();
        void                   commit_transaction();

        // basic literal parsing
        bool                   parse_bool( const std::string& );
        std::string            parse_string( const std::string& );
        double                 parse_double( const std::string& );

        /**
		 * Convert a Token to a Member.
	     * @return a Member* that owns _token.
	     */
		Member*                token_to_member(Token* _token);

		// those parse_XXXX() are parsing from the token_ribbon at current_position.
		// After a call, cursor may have moved or could have been reverted to initial position.
        ScopedCodeBlockNode*   parse_scope();
        InstructionNode*       parse_instruction();
        Member*                parse_variable_declaration();
        CodeBlockNode*         parse_code_block();
        ConditionalStructNode* parse_conditional_structure();
        ForLoopNode*           parse_for_loop();
        CodeBlockNode*         parse_program();
		Member*                parse_function_call();
		Member*                parse_parenthesis_expression();
		Member*                parse_binary_operator_expression(unsigned short _precedence = 0u, Member *_left = nullptr);

		/** To parse two tokens (ex: !true, -5, etc..) */
		Member*                parse_unary_operator_expression(unsigned short _precedence = 0u);

		/** To parse a primary expression (ex: "myVariable", "10.4", etc... ) */
		Member*                parse_atomic_expression();

		/** Build a graph recursively starting at the current cursor position. */
		Member*                parse_expression(unsigned short _precedence = 0u, Member *_left = nullptr);

		/** Split a given expression string into tokens (cf. Token) */
		bool                   tokenize_string(const std::string &_expression);

		/** Check if the token ribbon match with the language's syntax.
		 * tokenizeExpressionString() must be called first */
		bool                   is_syntax_valid();

		/** Get the current scope (during parsing, scope changes, we need to know the current to push any new variable) */
        ScopedCodeBlockNode*   get_current_scope();

        /** Given a Literal token, return its type */
        Type                   get_literal_type(const Token *_token)const;

    private:
		/** A language to get Semantic and Syntax (not yet implemented) */
		const Language* m_language;

		/** The target container of the parser in which all generated nodes will be pushed into*/
		GraphNode*      m_graph;

		/** A token ribbon to store a token list and be able to eat them easily */
		TokenRibbon     m_token_ribbon;
    };

}