#pragma once

#include <string>
#include <vector>
#include <stack>
#include <memory>
#include <exception>

#include "core/VariableNode.h"
#include "fw/core/reflection/reflection"
#include "fw/core/system.h"

#include "core/Token.h"
#include "core/TokenRibbon.h"

namespace ndbl{

    // forward declarations
    class ConditionalStructNode;
    class ForLoopNode;
    class IScope;
    class InstructionNode;
    class InvokableComponent;
    class Scope;
    class WhileLoopNode;
    class Graph;
    class Node;
    class Property;
    class VariableNode;

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
        bool                   tokenize(const std::string& _string);                 // Tokenize a string, return true for success. Tokens are stored in the token ribbon.
        bool                   tokenize(char* buffer, size_t buffer_size);           // Tokenize a buffer of a certain length, return true for success. Tokens are stored in the token ribbon.
        bool                   parse(const std::string& _in, Graph *_out);       // Try to convert a source code (input string) to a program tree (output graph). Return true if evaluation went well and false otherwise.
        Token                  parse_token(char *buffer, size_t buffer_size, size_t &global_cursor) const; // parse a single token from position _cursor in _string.
        Token                  parse_token(const std::string& _string) const;
        ID<Node>               parse_scope();                                         // Try to parse a scope.
        ID<InstructionNode>    parse_instr();                                         // Try to parse an instruction.
        Slot                   parse_variable_declaration();                          // Try to parse a variable declaration (ex: "int a = 10;").
        ID<Scope>              parse_code_block(ID<Scope> curr_scope);                       // Try to parse a code block with the option to create a scope or not (reusing the current one).
        ID<ConditionalStructNode> parse_conditional_structure();                  // Try to parse a conditional structure (if/else if/.else) recursively.
        ID<ForLoopNode>        parse_for_loop();                                      // Try to parse a "for loop".
        ID<WhileLoopNode>      parse_while_loop();                                    // Try to parse a "while loop".
        ID<Node>               parse_program();                                       // Try to parse an entire program.
        Slot                   parse_function_call();                                 // Try to parse a function call.
        Slot                   parse_parenthesis_expression();                        // Try to parse a parenthesis expression.
        Slot                   parse_unary_operator_expression(unsigned short _precedence = 0u);                               // Try to parse a unary expression.
        Slot                   parse_binary_operator_expression(unsigned short _precedence = 0u, Slot _left = {}); // Try to parse a binary expression.
        Slot                   parse_atomic_expression();                                                                      // Try to parse an atomic expression (ex: "1", "a")
        Slot                   parse_expression(unsigned short _precedence = 0u, Slot _left_override = {});                 // Try to parse an expression
        Slot                   parse_token(Token _token);
        bool                   to_bool(const std::string& );                          // convert a boolean string ("true"|"false") to a boolean.
        std::string            to_unquoted_string(const std::string& _quoted_str);    // convert a quoted string to a string.
        double                 to_double(const std::string& );                        // convert a double string (ex: "10.0") to a double.
        i16_t                  to_i16(const std::string& );                           // convert an integer string (ex: "42") to an integer.
    private:
        inline bool            allow_to_attach_suffix(Token_t type) const
        {
            return    type != Token_t::identifier          // identifiers must stay clean because they are reused
                   && type != Token_t::parenthesis_open    // ")" are lost when creating AST
                   && type != Token_t::parenthesis_close;  // "(" are lost when creating AST
        }
        void                   start_transaction();                                   // Start a parsing transaction. Must be followed by rollback_transaction or commit_transaction.
        void                   rollback_transaction();                                // Rollback the pending transaction (revert cursor to parse again from the transaction start).
        void                   commit_transaction();                                  // Commit the pending transaction
		bool                   is_syntax_valid();                                     // Check if the syntax of the token ribbon is correct. (ex: ["12", "-"] is incorrect)
        ID<Scope>              get_current_scope();                                   // Get the current scope. There is always a scope (main's scope program).
        ID<Node>               get_current_scope_node();
        static inline bool     is_letter(char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'); }
        static inline bool     is_digit(char c) { return c >= '0' && c <= '9'; }

    public:
        struct ParserState
        {
            struct
            {
                char* buffer;         // input, owned
                size_t size;
            } source;
            TokenRibbon        ribbon;
            Graph*             graph;         // output, not owned
            std::stack<ID<Scope>> scope;  // nested scopes

            ParserState();
            ~ParserState();
            void set_source_buffer(const char* str, size_t size);
            void clear();
        } parser_state;

    private: bool m_strict_mode; // When strict mode is ON, any use of undeclared symbol is rejected.
                                 // When OFF, parser can produce a graph with undeclared symbols but the compiler won't be able to handle it.

        // Serializer ------------------------------------------------------------------
    public:
        std::string&           serialize_invokable(std::string& _out, const InvokableComponent *_component) const;
        std::string&           serialize_func_call(std::string& _out, const fw::func_type *_signature, const std::vector<DirectedEdge> inputs)const;
        std::string&           serialize_func_sig(std::string& _out, const fw::func_type*)const;
        std::string&           serialize_token_t(std::string& _out, const Token_t&)const;
        std::string&           serialize_token(std::string& _out, const Token &) const;
        std::string&           serialize_type(std::string& _out, const fw::type*) const;
        std::string&           serialize_edge(std::string& _out, const DirectedEdge &_edge, bool recursively = true)const;   // serialize a property (with a recursive option if it has its input connected to another property).
        std::string&           serialize_instr(std::string& _out, const InstructionNode *_instruction)const;
        std::string&           serialize_node(std::string& _out, ID<const Node> _node)const;
        std::string&           serialize_scope(std::string& _out, const Scope *_scope)const;
        std::string&           serialize_for_loop(std::string& _out, const ForLoopNode *_for_loop)const;
        std::string&           serialize_while_loop(std::string& _out, const WhileLoopNode *_while_loop_node)const;
        std::string&           serialize_cond_struct(std::string& _out, const ConditionalStructNode *_condStruct) const;
        std::string&           serialize_variant(std::string& _out, const fw::variant*) const;
        std::string&           serialize_variable(std::string& _out, const VariableNode *_node) const;

        // Language definition -------------------------------------------------------------------------
    public:
        invokable_ptr                   find_function(const fw::func_type*) const;                   // Find a function by signature (strict first, then cast allowed)
        invokable_ptr                   find_function_exact(const fw::func_type*) const;             // Find a function by signature (no cast allowed).
        invokable_ptr                   find_function_fallback(const fw::func_type*) const;          // Find a function by signature (casts allowed).
        invokable_ptr                   find_operator_fct(const fw::func_type*) const;               // Find an operator's function by signature (strict first, then cast allowed)
        invokable_ptr                   find_operator_fct_exact(const fw::func_type*) const;         // Find an operator's function by signature (no cast allowed).
        invokable_ptr                   find_operator_fct_fallback(const fw::func_type*) const;      // Find an operator's function by signature (casts allowed).
        const fw::Operator*             find_operator(const std::string& , fw::Operator_t) const;    // Find an operator by symbol and type (unary, binary or ternary).
        const Invokable_vec &           get_api()const { return m_functions; }                   // Get all the functions registered in the language. (TODO: why do we store the declared functions here? can't we load them in the VirtualMachine instead?).
        std::string&                    to_string(std::string& /*out*/, const fw::type*)const;   // Convert a type to string (by ref).
        std::string&                    to_string(std::string& /*out*/, Token_t)const;           // Convert a type to a token_t (by ref).
        std::string                     to_string(const fw::type *) const;                       // Convert a type to string.
        std::string                     to_string(Token_t)const;                                 // Convert a type to a token_t.
        const fw::type*                 get_type(Token_t _token)const;                           // Get the type corresponding to a given token_t (must be a type keyword)
        void                            add_function(std::shared_ptr<const fw::iinvokable>);     // Adds a new function (regular or operator's implementation).
        int                             get_precedence(const fw::iinvokable*)const;              // Get the precedence of a given function (precedence may vary because function could be an operator implementation).

        template<typename T> void       load_library(); // Instantiate a library from its type (uses reflection to get all its static methods).

    private:
        struct {
            std::vector<std::tuple<const char*, Token_t>>                  keywords;
            std::vector<std::tuple<const char*, Token_t, const fw::type*>> types;
            std::vector<std::tuple<const char*, fw::Operator_t, int>>      operators;
            std::vector<std::tuple<char, Token_t>>                         chars;
        } m_definition; // language definition

        operators_vec m_operators;                                             // the allowed operators (!= implementations).
        Invokable_vec m_operators_impl;                                        // operators' implementations.
        Invokable_vec m_functions;                                             // all the functions (including operator's).
        std::unordered_map<Token_t, char>        m_single_char_by_keyword;
        std::unordered_map<Token_t, const char*> m_keyword_by_token_t;         // token_t to string (ex: Token_t::keyword_double => "double").
        std::unordered_map<fw::type::id_t, const char*>   m_keyword_by_type_id;
        std::unordered_map<char, Token_t>                 m_token_t_by_single_char;
        std::unordered_map<size_t, Token_t>               m_token_t_by_keyword;     // keyword reserved by the language (ex: int, string, operator, if, for, etc.)
        std::unordered_map<fw::type::id_t, Token_t>       m_token_t_by_type_id;
        std::unordered_map<Token_t, const fw::type*>      m_type_by_token_t;        // token_t to type. Works only if token_t refers to a type keyword.
        std::string &serialize_property(std::string &_out, const Property *pProperty) const;
    };

    template<typename T>
    void Nodlang::load_library()
    {
        T library; // will force static code to run

        auto type = fw::type::get<T>();
        for(auto& each_static : type->get_static_methods())
        {
            add_function(each_static);
        }
    }
}

