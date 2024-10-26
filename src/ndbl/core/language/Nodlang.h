#pragma once

#include <string>
#include <vector>
#include <stack>
#include <exception>

#include "tools/core/reflection/reflection"
#include "tools/core/System.h"
#include "tools/core/hash.h"
#include "tools/core/Optional.h"

#include "ndbl/core/VariableNode.h"
#include "ndbl/core/Token.h"
#include "ndbl/core/TokenRibbon.h"
#include "ndbl/core/Graph.h"

namespace ndbl{

    // forward declarations
    class IfNode;
    class ForLoopNode;
    class IScope;
    class InstructionNode;
    class FunctionNode;
    class Scope;
    class WhileLoopNode;
    class Node;
    class Property;
    class VariableNode;
    class VariableRefNode;

    typedef int SerializeFlags;
    enum SerializeFlag_
    {
        SerializeFlag_NONE             = 0,
        SerializeFlag_RECURSE          = 1 << 0,
        SerializeFlag_WRAP_WITH_BRACES = 1 << 1
    };

    /**
	 * @class Nodlang is Nodable's language.
	 * This class allows to parse, serialize, and define Nodlang language.
	 */
	class Nodlang
    {
	public:
        explicit Nodlang(bool _strict = false);
		~Nodlang();

        // Parser ---------------------------------------------------------------------
        bool                            tokenize(); // tokenise from curent parser state
        bool                            tokenize(const std::string& _string); // Tokenize a string, return true for success. Tokens are stored in the token ribbon.
        bool                            parse(Graph* graph_out, const std::string& code_in); // Try to convert a source code (input string) to a program tree (output graph). Return true if evaluation went well and false otherwise.
        tools::Optional<Node*>          parse_program();
        Token                           parse_token(const std::string& _string) const;
        Token                           parse_token(const char *buffer, size_t buffer_size, size_t &global_cursor) const; // parse a single token from position _cursor in _string.
        tools::Optional<Node*>          parse_scope( Slot* parent_slot_out );
        tools::Optional<Node*>          parse_single_scope_or_instruction(Slot* child_slot );
        tools::Optional<Node*>          parse_instr();
        tools::Optional<Slot*>          parse_variable_declaration(); // Try to parse a variable declaration (ex: "int a = 10;").
        void                            parse_code_block(); // Try to parse a code block with the option to create a scope or not (reusing the current one).
        tools::Optional<IfNode*>        parse_conditional_structure(); // Try to parse a conditional structure (if/else if/.else) recursively.
        tools::Optional<ForLoopNode*>   parse_for_loop();
        tools::Optional<WhileLoopNode*> parse_while_loop();
        tools::Optional<Slot*>          parse_function_call();
        tools::Optional<Slot*>          parse_parenthesis_expression();
        tools::Optional<Slot*>          parse_unary_operator_expression(u8_t _precedence = 0);
        tools::Optional<Slot*>          parse_binary_operator_expression(u8_t _precedence, Slot* _left);
        tools::Optional<Slot*>          parse_atomic_expression();
        tools::Optional<Slot*>          parse_expression(u8_t _precedence = 0, tools::Optional<Slot*> _left_override = nullptr);
        tools::Optional<Slot*>          token_to_slot(Token _token);
        bool                            parse_bool_or(const std::string&, bool default_value ) const;
        double                          parse_double_or(const std::string&, double default_value ) const;
        int                             parse_int_or(const std::string&, int default_value ) const;
        std::string                     remove_quotes(const std::string& _quoted_str) const;

    private:
        bool                            accepts_suffix(Token_t type) const;
		bool                            is_syntax_valid(); // Check if the syntax of the token ribbon is correct. (ex: ["12", "-"] is incorrect)

    public:
        struct ParserState
        {
            const char*         buffer() const { ASSERT(_buffer.data); return _buffer.data; }
            size_t              buffer_size() const { return _buffer.size; }
            void                reset_ribbon(const char* new_buf = nullptr, size_t new_size = 0);
            void                reset_graph(Graph* new_graph);
            void                reset_scope_stack();
            std::string         string() const { return _ribbon.to_string(); }; // Ribbon's
            Graph*              graph() const { ASSERT(_graph); return _graph; }
            TokenRibbon&        tokens()  { return _ribbon; }
            Scope*              current_scope() const { ASSERT( !_scope.empty() ); return _scope.top(); }
            Node*               current_scope_node() const { return _scope.top()->get_owner(); };
            void                push_scope(Scope* scope) { _scope.push( scope); };
            void                pop_scope() { _scope.pop(); };
            const char*         buffer_at(size_t offset) { ASSERT(offset < _buffer.size ); return _buffer.data + offset; }
            void                start_transaction() { _ribbon.start_transaction(); }
            void                commit() { _ribbon.commit(); }
            void                rollback() { _ribbon.rollback(); }
        private:
            struct Buffer
            {
                const char* data = nullptr; // NOT owned
                size_t      size = 0;
            };

            Buffer              _buffer;
            Graph*              _graph = nullptr; // NOT owned
            TokenRibbon         _ribbon;
            std::stack<Scope*>  _scope; // nested scopes
        } parser_state;

    private: bool m_strict_mode; // When strict mode is ON, any use of undeclared symbol is rejected.
                                 // When OFF, parser can produce a graph with undeclared symbols but the compiler won't be able to handle it.

        // Serializer ------------------------------------------------------------------
    public:
        std::string& serialize_bool(std::string& _out, bool b) const;
        std::string& serialize_int(std::string& _out, int i) const;
        std::string& serialize_double(std::string& _out, double d) const;
        std::string& serialize_invokable(std::string&_out, const FunctionNode*) const;
        std::string& serialize_invokable_sig(std::string& _out, const tools::IInvokable*)const;
        std::string& serialize_func_call(std::string& _out, const tools::FunctionDescriptor *_signature, const std::vector<Slot*>& inputs)const;
        std::string& serialize_func_sig(std::string& _out, const tools::FunctionDescriptor*)const;
        std::string& serialize_token_t(std::string& _out, const Token_t)const;
        std::string  serialize_token_t(Token_t _token)const;
        std::string& serialize_token(std::string& _out, const Token &) const;
        std::string& serialize_type(std::string& _out, const tools::TypeDescriptor*) const;
        std::string  serialize_type(const tools::TypeDescriptor *_type) const;
        std::string& serialize_input(std::string& _out, const Slot *_slot, SerializeFlags _flags = SerializeFlag_NONE )const;
        std::string& serialize_output(std::string& _out, const Slot *_slot, SerializeFlags _flags = SerializeFlag_NONE )const;
        std::string& _serialize_node(std::string &_out, const Node* node, SerializeFlags _flags = SerializeFlag_NONE ) const;
        std::string& serialize_scope(std::string& _out, const Scope *_scope)const;
        std::string& serialize_for_loop(std::string& _out, const ForLoopNode *_for_loop)const;
        std::string& serialize_while_loop(std::string& _out, const WhileLoopNode *_while_loop_node)const;
        std::string& serialize_cond_struct(std::string& _out, const IfNode*_condition_struct ) const;
        std::string& serialize_variable(std::string& _out, const VariableNode*) const;
        std::string& serialize_variable_ref(std::string &_out, const VariableRefNode *_node) const;
        std::string& serialize_property(std::string &_out, const Property*) const;

        // Language definition -------------------------------------------------------------------------

    private:
        const tools::IInvokable* find_function(u32_t _hash) const;
    public:
        const tools::IInvokable* find_function(const char* _signature ) const;           // Find a function by signature as string (ex:   "int multiply(int,int)" )
        const tools::IInvokable* find_function(const tools::FunctionDescriptor*) const;               // Find a function by signature (strict first, then cast allowed)
        const tools::IInvokable* find_function_exact(const tools::FunctionDescriptor*) const;         // Find a function by signature (no cast allowed).
        const tools::IInvokable* find_function_fallback(const tools::FunctionDescriptor*) const;      // Find a function by signature (casts allowed).
        const tools::IInvokable* find_operator_fct(const tools::FunctionDescriptor*) const;           // Find an operator's function by signature (strict first, then cast allowed)
        const tools::IInvokable* find_operator_fct_exact(const tools::FunctionDescriptor*) const;     // Find an operator's function by signature (no cast allowed).
        const tools::IInvokable* find_operator_fct_fallback(const tools::FunctionDescriptor*) const;  // Find an operator's function by signature (casts allowed).
        const tools::Operator* find_operator(const std::string& , tools::Operator_t) const;// Find an operator by symbol and type (unary, binary or ternary).
        const std::vector<const tools::IInvokable*>& get_api()const { return m_functions; } // Get all the functions registered in the language.
        Token_t               to_literal_token(const tools::TypeDescriptor*) const;
        const tools::TypeDescriptor*    get_type(Token_t _token)const;                              // Get the type corresponding to a given token_t (must be a type keyword)
        void                  add_function(const tools::IInvokable*);                     // Adds a new function (regular or operator's implementation).
        int                   get_precedence(const tools::FunctionDescriptor*)const;                // Get the precedence of a given function (precedence may vary because function could be an operator implementation).

        template<typename T> void load_library(); // Instantiate a library from its type (uses reflection to get all its static methods).
    private:
        struct {
            std::vector<std::tuple<const char*, Token_t>>                  keywords;
            std::vector<std::tuple<const char*, Token_t, const tools::TypeDescriptor*>> types;
            std::vector<std::tuple<const char*, tools::Operator_t, int>>      operators;
            std::vector<std::tuple<char, Token_t>>                         chars;
        } m_definition; // language definition

        std::vector<const tools::Operator*>               m_operators;                // the allowed operators (!= implementations).
        std::vector<const tools::IInvokable*>             m_operators_impl;           // operators' implementations.
        std::vector<const tools::IInvokable*>             m_functions;                // all the functions (including operator's).
        std::unordered_map<u32_t , const tools::IInvokable*> m_functions_by_signature; // Functions indexed by signature hash
        std::unordered_map<Token_t, char>                 m_single_char_by_keyword;
        std::unordered_map<Token_t, const char*>          m_keyword_by_token_t;       // token_t to string (ex: Token_t::keyword_double => "double").
        std::unordered_map<std::type_index, const char*>  m_keyword_by_type_id;
        std::unordered_map<char, Token_t>                 m_token_t_by_single_char;
        std::unordered_map<size_t, Token_t>               m_token_t_by_keyword;       // keyword reserved by the language (ex: int, string, operator, if, for, etc.)
        std::unordered_map<std::type_index, Token_t>      m_token_t_by_type_id;
        std::unordered_map<Token_t, const tools::TypeDescriptor*>   m_type_by_token_t;          // token_t to type. Works only if token_t refers to a type keyword.

    };

    template<typename T>
    void Nodlang::load_library()
    {
        T library; // Libraries are static and this will force static code to run. TODO: add load/release methods

        auto class_desc = tools::type::get_class<T>();
        for(const tools::IInvokable* method : class_desc->get_statics() )
        {
            add_function(method);
        }
    }

    [[nodiscard]]
    Nodlang* init_language();
    Nodlang* get_language();
    void     shutdown_language(Nodlang*); // undo init_language()
}

