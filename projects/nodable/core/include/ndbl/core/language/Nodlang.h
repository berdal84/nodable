#pragma once

#include <string>
#include <vector>
#include <stack>
#include <memory>
#include <exception>
#include <fw/core/reflection/reflection>
#include "fw/core/System.h"

#include <ndbl/core/ForLoopNode.h>
#include <ndbl/core/Token.h>
#include <ndbl/core/TokenRibbon.h>

namespace ndbl{

    // forward declarations
    class ConditionalStructNode;
    class ForLoopNode;
    class IScope;
    class InstructionNode;
    class InvokableComponent;
    class Scope;

	/**
	 * @class Nodlang is Nodable's language.
	 * This class allows to parse, serialize, and define Nodlang language.
	 */
	class Nodlang
    {
	public:
        using operators_vec = std::vector<const fw::Operator*>;
        using invokable_ptr = std::shared_ptr<const fw::iinvokable>;
        using Invokable_vec = std::vector<std::shared_ptr<const fw::iinvokable>>;

        explicit Nodlang(bool _strict = false);
		~Nodlang() = default;
        static Nodlang& get_instance();

        // Parser ---------------------------------------------------------------------
    public:
        bool                   parse(const std::string& _in, GraphNode *_out);       // Try to convert a source code (input string) to a program tree (output graph). Return true if evaluation went well and false otherwise.
    private:
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
        GraphNode*             m_graph;           // current graph output.
        TokenRibbon            m_token_ribbon;    // This token ribbon is cleared/filled when tokenize() is called.
        std::stack<Scope*>     m_scope_stack;     // Current scopes (babushka dolls).
        bool                   m_strict_mode;     // When strict mode is ON, any use of undeclared variable is rejected. When OFF, parser can produce a graph with undeclared variables but the compiler won't be able to handle it.

        // Serializer ------------------------------------------------------------------
    public:
        std::string&           serialize(std::string& _out, const InvokableComponent*) const;
        std::string&           serialize(std::string& _out, const fw::func_type*, const std::vector<Property *>&)const;  // serialize a function call with arguments.
        std::string&           serialize(std::string& _out, const fw::func_type*)const;                                  // serialize a function signature.
        std::string&           serialize(std::string& _out, const Token_t&)const;
        std::string&           serialize(std::string& _out, std::shared_ptr<const Token>) const;
        std::string&           serialize(std::string& _out, fw::type) const;
        std::string&           serialize(std::string& _out, const Property *, bool recursively = true)const;   // serialize a property (with a recursive option if it has its input connected to another property).
        std::string&           serialize(std::string& _out, const InstructionNode*)const;
        std::string&           serialize(std::string& _out, const Node*)const;
        std::string&           serialize(std::string& _out, const Scope*)const;
        std::string&           serialize(std::string& _out, const ForLoopNode* _for_loop)const;
        std::string&           serialize(std::string& _out, const ConditionalStructNode*) const;
        std::string&           serialize(std::string& _out, const fw::variant* variant) const;
        std::string&           serialize(std::string& _out, const VariableNode *_node) const;    // serialize a variable (declared or not)

        // Language definition -------------------------------------------------------------------------
    public:
        invokable_ptr                   find_function(const fw::func_type*) const;                   // Find a function by signature.
        invokable_ptr                   find_operator_fct(const fw::func_type*) const;               // Find an operator's function by signature (casts allowed).
        invokable_ptr                   find_operator_fct_exact(const fw::func_type*) const;         // Find an operator's function by signature (strict mode, no cast allowed).
        const fw::Operator*             find_operator(const std::string& , fw::Operator_t) const;    // Find an operator by symbol and type (unary, binary or ternary).
        const Invokable_vec &           get_api()const { return m_functions; }                   // Get all the functions registered in the language. (TODO: why do we store the declared functions here? can't we load them in the VirtualMachine instead?).
        const std::vector<std::regex>&  get_token_regexes()const { return m_token_regex;  }      // Get all the regexes to convert a string to the corresponding token_t.
        std::string&                    to_string(std::string& /*out*/, const fw::type&)const;   // Convert a type to string (by ref).
        std::string&                    to_string(std::string& /*out*/, Token_t)const;           // Convert a type to a token_t (by ref).
        std::string                     to_string(fw::type) const;                               // Convert a type to string.
        std::string                     to_string(Token_t)const;                                 // Convert a type to a token_t.
        fw::type                        get_type(Token_t _token)const;                           // Get the type corresponding to a given token_t (must be a type keyword)
        const std::vector<Token_t>&     get_token_t_by_regex_index()const { return m_token_t_by_regex_index; }
        void                            add_function(std::shared_ptr<const fw::iinvokable>);     // Adds a new function (regular or operator's implementation).
        int                             get_precedence(const fw::iinvokable*)const;              // Get the precedence of a given function (precedence may vary because function could be an operator implementation).

    private:
        template<typename T>
        void load_library()             // Instantiate a library from its type (uses reflection to get all its static methods).
        {
            T library; // will force static code to run

            auto type = fw::type::get<T>();
            for(auto& each_static : type.get_static_methods())
            {
                add_function(each_static);
            }
        }

        void                            add_operator(const char*, fw::Operator_t, int _precedence);  // Add a new (abstract) operator (ex: "==", Operator_t::binary, 5)
        invokable_ptr                   find_operator_fct_fallback(const fw::func_type*) const;      // Find a fallback operator function for a given signature (allows cast).
        void                            add_regex(const std::regex&, Token_t);                   // Register a regex to token_t couple.
        void                            add_regex(const std::regex&, Token_t, fw::type);         // Register a regex, token_t, type tuple.
        void                            add_string(std::string, Token_t);                        // Register a string to token_t couple. ( "<<" => Token_t::operator )
        void                            add_char(char, Token_t);                                 // Register a char to token_t couple ( '*' => Token_t::operator )
        void                            add_type(fw::type, Token_t, std::string);                // Register a type, token_t, string tuple (ex: type::get<int>, Token_t::keyword_int, "int")

        operators_vec m_operators;                                             // the allowed operators (!= implementations).
        Invokable_vec m_operators_impl;                                        // operators' implementations.
        Invokable_vec m_functions;                                             // all the functions (including operator's).
        std::vector<std::regex>  m_type_regex;                                 // Regular expressions to get a type from a string.
        std::vector<fw::type>    m_type_by_regex_index;                        // Types sorted by m_type_regex index.
        std::vector<std::regex>  m_token_regex;                                // Regular expressions to get a token from a string.
        std::vector<Token_t>     m_token_t_by_regex_index;                     // Token sorted by m_token_regex index.
        std::unordered_map<size_t, Token_t>      m_type_hashcode_to_token_t;   // type's hashcode into a token_t (ex: type::get<std::string>().hashcode() => Token_t::keyword_string)
        std::unordered_map<size_t, std::string>  m_type_hashcode_to_string;    // type's hashcode into a string (ex: type::get<std::string>().hashcode() => "std::string")
        std::unordered_map<Token_t, fw::type>    m_token_type_keyword_to_type; // token_t to type. Works only if token_t refers to a type keyword.
        std::unordered_map<Token_t, const char>  m_token_t_to_char;            // token_t to single char (ex: Token_t::operator => '*').
        std::unordered_map<Token_t, std::string> m_token_t_to_string;          // token_t to string (ex: Token_t::keyword_double => "double").
        std::unordered_map<size_t, Token_t>      m_char_to_token_t;            // single char to token_t (ex: '*' => Token_t::operator).
    };
}