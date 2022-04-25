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
#include <nodable/core/IParser.h>

namespace ndbl{

	class NodableLanguage;

	/**
		The role of this class is to convert code string to a Nodable graph.

		The main strategy is:
		- cut string into tokens
		- parse token list to convert it to Nodable graph.

		ex: "a+b" will became an Add node connected to two VariableNode* a and b.

	    There is no AST (Abstract Syntax Tree) since Nodable keep graph (Nodes) linked to text (tokens) all the time.
	*/

	class NodableParser : public IParser
	{
	public:
        NodableParser(const NodableLanguage& _lang, bool _strict = false );
		~NodableParser() override {}

		/** Try to convert a source code to a program tree.
		   Return true if evaluation went well and false otherwise. */
		bool                   parse(const std::string& _in, GraphNode *_out) override;

    protected:

	    /** Start a parsing transaction.
	     * This will memorise the current cursor position on the token ribbon.
	     * A commit/rollback must follow. */
        void                   start_transaction() override;
        void                   rollback_transaction() override;
        void                   commit_transaction() override;
        bool                   to_bool(const std::string& ) override;
        std::string            to_string(const std::string& _quoted_str) override;
        double                 to_double(const std::string& ) override;
        i16_t                  to_i16(const std::string& ) override;
		Member*                to_member(std::shared_ptr<Token> _token) override;
		Node*                  parse_scope() override;
        InstructionNode*       parse_instr() override;
        Member*                parse_variable_declaration() override;
        IScope*                parse_code_block(bool _create_scope) override;
        ConditionalStructNode* parse_conditional_structure() override;
        ForLoopNode*           parse_for_loop() override;
        Node*                  parse_program() override;
		Member*                parse_function_call() override;
		Member*                parse_parenthesis_expression() override;
		Member*                parse_binary_operator_expression(unsigned short _precedence = 0u, Member *_left = nullptr) override;
		Member*                parse_unary_operator_expression(unsigned short _precedence = 0u) override;
		Member*                parse_atomic_expression() override;
		Member*                parse_expression(unsigned short _precedence = 0u, Member *_left = nullptr) override;
		bool                   tokenize(const std::string& _string) override;
		bool                   is_syntax_valid() override;
        Scope*                 get_current_scope() override;

    private:
		const NodableLanguage& m_language;
		GraphNode*             m_graph;
		TokenRibbon            m_token_ribbon;
		std::stack<Scope*>     m_scope_stack;
        bool                   m_strict_mode;
    };

}