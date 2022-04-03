#pragma once

#include <string>
#include <vector>
#include <stack>
#include <memory>
#include <exception>

#include <nodable/core/types.h> // forward declarations
#include <nodable/core/Language.h>
#include <nodable/core/Token.h>
#include <nodable/core/TokenRibbon.h>
#include <nodable/core/ForLoopNode.h>
#include <nodable/core/reflection/R.h>

namespace Nodable{

    // forward declaration
    class InstructionNode;
    class Scope;
    class ForLoopNode;

	/**
		The role of this class is to convert code string to a Nodable graph.

		The main strategy is:
		- cut string into tokens
		- parse token list to convert it to Nodable graph.

		ex: "a+b" will became an Add node connected to two VariableNode* a and b.

	    There is no AST (Abstract Syntax Tree) since Nodable keep graph (Nodes) linked to text (tokens) all the time.
	*/

	class
    Parser
	{
	public:
   		explicit Parser(const Language* _lang, bool _strict = false )
            : m_language(_lang)
            , m_strict_mode(_strict)
            , m_graph(nullptr){}
		~Parser(){}

		/** Try to convert a source code to a program tree.
		   Return true if evaluation went well and false otherwise. */
		bool                   parse_graph(const std::string &_source_code, GraphNode *_graphNode);

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
        double                 parse_int16( const std::string& );

        /**
		 * Convert a Token to a Member.
	     * @return a Member* that owns _token.
	     */
		Member*                token_to_member(std::shared_ptr<Token> _token);

		// those parse_XXXX() are parsing from the token_ribbon at current_position.
		// After a call, cursor may have moved or could have been reverted to initial position.
		Node*                  parse_scope();
        InstructionNode*       parse_instr();
        Member*                parse_variable_declaration();
        IScope*                parse_code_block(bool _create_scope);
        ConditionalStructNode* parse_conditional_structure();
        ForLoopNode*           parse_for_loop();
        Node*                  parse_program();
		Member*                parse_function_call();
		Member*                parse_parenthesis_expression();
		Member*                parse_binary_operator_expression(unsigned short _precedence = 0u, Member *_left = nullptr);

		/** To parse two tokens (ex: !true, -5, etc..) */
		Member*                parse_unary_operator_expression(unsigned short _precedence = 0u);

		/** To parse a primary expression (ex: "myVariable", "10.4", etc... ) */
		Member*                parse_atomic_expression();

		/** Build a graph recursively starting at the current cursor position. */
		Member*                parse_expression(unsigned short _precedence = 0u, Member *_left = nullptr);

		/** Split a given code_source to tokens (cf. Token) */
		bool                   tokenize_string(const std::string& _code_source_portion);

		/** Check if the token ribbon match with the language's syntax.
		 * tokenizeExpressionString() must be called first */
		bool                   is_syntax_valid();

		/** Get the current scope (during parsing, scope changes, we need to know the current to push any new variable) */
        Scope*                 get_current_scope();

        /** Given a Literal token, return its type */
        R::Type            get_literal_type(std::shared_ptr<const Token> _token)const;

    private:
		/** A language to get Semantic and Syntax (not yet implemented) */
		const Language* m_language;

		/** The target container of the parser in which all generated nodes will be pushed into*/
		GraphNode*      m_graph;

		/** A token ribbon to store a token list and be able to eat them easily */
		TokenRibbon     m_token_ribbon;

		/** Scope stack */
		std::stack<Scope*> m_scope_stack;

		/** In strict mode, parsing fails earlier.
		 * Ex: if trying to parse "double b = a + c;" it will fail when trying to get a reference for variable a. */
        bool m_strict_mode;
    };

}