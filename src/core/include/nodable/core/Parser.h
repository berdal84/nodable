#pragma once

#include <string>
#include <vector>
#include <stack>
#include <memory>
#include <exception>

#include <nodable/core/types.h> // forward declarations
#include <nodable/core/Token.h>
#include <nodable/core/TokenRibbon.h>
#include <nodable/core/ForLoopNode.h>
#include <nodable/core/reflection/reflection>

namespace Nodable{

    // forward declaration
    class ConditionalStructNode;
    class InstructionNode;
    class ForLoopNode;
    class Language;
    class IScope;
    class Scope;

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
        Parser(const Language* _lang, bool _strict = false );
		~Parser(){}

		/** Try to convert a source code to a program tree.
		   Return true if evaluation went well and false otherwise. */
		bool                   parse(const std::string& _in, GraphNode *_out);

    protected:

	    /** Start a parsing transaction.
	     * This will memorise the current cursor position on the token ribbon.
	     * A commit/rollback must follow. */
        void                   start_transaction();
        void                   rollback_transaction();
        void                   commit_transaction();

        static bool            to_bool(const std::string& );
        static std::string     to_string(const std::string& _quoted_str);
        static double          to_double(const std::string& );
        static i16_t           to_i16(const std::string& );
		Member*                to_member(std::shared_ptr<Token> _token);

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
		Member*                parse_unary_operator_expression(unsigned short _precedence = 0u);
		Member*                parse_atomic_expression();
		Member*                parse_expression(unsigned short _precedence = 0u, Member *_left = nullptr);

		bool                   tokenize(const std::string& _string);
		bool                   is_syntax_valid();
        Scope*                 get_current_scope();

    private:
		const Language&    m_language;
		GraphNode*         m_graph;
		TokenRibbon        m_token_ribbon;
		std::stack<Scope*> m_scope_stack;
        bool               m_strict_mode;
    };

}