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

namespace ndbl{

    // forward declarations
	class NodableLanguage;
    class ConditionalStructNode;
    class IScope;

	/**
		The role of this class is to convert source code to a graph (a syntax tree in this case).

		The main strategy is:
		- cut string into tokens
		- parse token list to convert it to Nodable graph.

		ex: "a+b" will became an Add node connected to two VariableNode* a and b.

	    There is no AST (Abstract Syntax Tree) since Nodable keep graph (Nodes) linked to text (tokens) all the time.
	*/

	class NodableParser
	{
	public:
        NodableParser(const NodableLanguage& _lang, bool _strict = false );
		~NodableParser() {}

		bool                   parse(const std::string& _in, GraphNode *_out);       // Try to convert a source code (input string) to a program tree (output graph). Return true if evaluation went well and false otherwise.

    protected:

        void                   start_transaction();                                   // Start a parsing transaction. Must be followed by rollback_transaction or commit_transaction.
        void                   rollback_transaction();                                // Rollback the pending transaction (revert cursor to parse again from the transaction start).
        void                   commit_transaction();                                  // Commit the pending transaction
        bool                   to_bool(const std::string& );                          // convert a boolean string ("true"|"false") to a boolean.
        std::string            to_unquoted_string(const std::string& _quoted_str);    // convert a quoted string to a string.
        double                 to_double(const std::string& );                        // convert a double string (ex: "10.0") to a double.
        i16_t                  to_i16(const std::string& );                           // convert an integer string (ex: "42") to an integer.
        Property *             to_property(std::shared_ptr<Token> _token);            // convert a token to a property.
		Node*                  parse_scope();                                         // Try to parse a scope.
        InstructionNode*       parse_instr();                                         // Try to parse an instruction.
        Property *             parse_variable_declaration();                          // Try to parse a variable declaration (ex: "int a = 10;").
        IScope*                parse_code_block(bool _create_scope);                  // Try to parse a code block with the option to create a scope or not (reusing the current one).
        ConditionalStructNode* parse_conditional_structure();                         // Try to parse a conditional structure (if/else if/.../else) recursively.
        ForLoopNode*           parse_for_loop();                                      // Try to parse a "for" loop.
        Node*                  parse_program();                                       // Try to parse an entire program.
        Property *             parse_function_call();                                 // Try to parse a function call.
        Property *             parse_parenthesis_expression();                        // Try to parse a parenthesis expression.
        Property *             parse_unary_operator_expression(unsigned short _precedence = 0u);                               // Try to parse a unary expression.
        Property *             parse_binary_operator_expression(unsigned short _precedence = 0u, Property *_left = nullptr);   // Try to parse a binary expression.
        Property *             parse_atomic_expression();                                                                      // Try to parse an atomic expression (ex: "1", "a")
        Property *             parse_expression(unsigned short _precedence = 0u, Property *_left = nullptr);                   // Try to parse an expression
		bool                   tokenize(const std::string& _string);                  // Tokenize a string, return true for success. Tokens are stored in the token ribbon.
		bool                   is_syntax_valid();                                     // Check if the syntax of the token ribbon is correct. (ex: ["12", "-"] is incorrect)
        Scope*                 get_current_scope();                                   // Get the current scope. There is always a scope (main's scope program).

    private:
		const NodableLanguage& m_language;        // The language definition to parse.
		GraphNode*             m_graph;           // current graph output.
		TokenRibbon            m_token_ribbon;    // This token ribbon is cleared/filled when tokenize() is called.
		std::stack<Scope*>     m_scope_stack;     // Scope of the current stacks (babushka dolls).
        bool                   m_strict_mode;     // When strict mode is ON, any use of undeclared variable is rejected. When OFF, parser can produce a graph with undeclared variables but the compiler won't be able to handle it.
    };
}