#pragma once

#include <string>
#include <vector>
#include <stack>
#include <memory>
#include <exception>

#include "tools/core/reflection/reflection"
#include "tools/core/system.h"
#include "tools/core/hash.h"

#include "ndbl/core/VariableNode.h"
#include "ndbl/core/Token.h"
#include "ndbl/core/TokenRibbon.h"

namespace ndbl{

    // forward declarations
    class IfNode;
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
        using operators_vec = std::vector<const tools::Operator*>;
        using invokable_ptr = std::shared_ptr<const tools::IInvokable>;
        using Invokable_vec = std::vector<std::shared_ptr<const tools::IInvokable>>;

        explicit Nodlang(bool _strict = false);
		~Nodlang();

        // Parser ---------------------------------------------------------------------
        bool                          tokenize(const std::string& _string);       // Tokenize a string, return true for success. Tokens are stored in the token ribbon.
        bool                          tokenize(char* buffer, size_t buffer_size); // Tokenize a buffer of a certain length, return true for success. Tokens are stored in the token ribbon.
        bool                          parse(const std::string& _in, Graph *_out); // Try to convert a source code (input string) to a program tree (output graph). Return true if evaluation went well and false otherwise.
        Token                         parse_token(char *buffer, size_t buffer_size, size_t &global_cursor) const; // parse a single token from position _cursor in _string.
        Token                         parse_token(const std::string& _string) const;
        Node*                         parse_scope( Slot& _parent_scope_slot );
        Node*                         parse_instr();
        Slot*                         parse_variable_declaration(); // Try to parse a variable declaration (ex: "int a = 10;").
        void                          parse_code_block(); // Try to parse a code block with the option to create a scope or not (reusing the current one).
        IfNode*                       parse_conditional_structure(); // Try to parse a conditional structure (if/else if/.else) recursively.
        ForLoopNode*                  parse_for_loop();
        WhileLoopNode*                parse_while_loop();
        Node*                         parse_program();
        Slot*                         parse_function_call();
        Slot*                         parse_parenthesis_expression();
        Slot*                         parse_unary_operator_expression(u8_t _precedence = 0);
        Slot*                         parse_binary_operator_expression(u8_t _precedence, Slot& _left);
        Slot*                         parse_atomic_expression();
        Slot*                         parse_expression(u8_t _precedence = 0, Slot* _left_override = nullptr);
        Slot*                         parse_token(Token _token);
        bool                          to_bool(const std::string& );
        std::string                   to_unquoted_string(const std::string& _quoted_str);
        double                        to_double(const std::string& );
    private:
        bool                          allow_to_attach_suffix(Token_t type) const;
        void                          start_transaction(); // Start a parsing transaction. Must be followed by rollback_transaction or commit_transaction.
        void                          rollback_transaction(); // Rollback the pending transaction (revert cursor to parse again from the transaction start).
        void                          commit_transaction(); // Commit the pending transaction
		bool                          is_syntax_valid(); // Check if the syntax of the token ribbon is correct. (ex: ["12", "-"] is incorrect)
        Scope*                        get_current_scope();
        Node*                         get_current_scope_node();

    public:
        struct ParserState
        {
            char*                     source_buffer;      // owned
            size_t                    source_buffer_size;
            TokenRibbon               ribbon;
            Graph*                    graph;              // not owned
            std::stack<Scope*>        scope;              // nested scopes

            ParserState();
            ~ParserState();
            void set_source_buffer(const char* str, size_t size);
            void clear();
        } parser_state;

    private: bool m_strict_mode; // When strict mode is ON, any use of undeclared symbol is rejected.
                                 // When OFF, parser can produce a graph with undeclared symbols but the compiler won't be able to handle it.

        // Serializer ------------------------------------------------------------------
    public:
        std::string& serialize_invokable(std::string&_out, const InvokableComponent &_component) const;
        std::string& serialize_func_call(std::string& _out, const tools::func_type *_signature, const std::vector<Slot*>& inputs)const;
        std::string& serialize_func_sig(std::string& _out, const tools::func_type*)const;
        std::string& serialize_token_t(std::string& _out, const Token_t&)const;
        std::string& serialize_token(std::string& _out, const Token &) const;
        std::string& serialize_type(std::string& _out, const tools::type*) const;
        std::string& serialize_input(std::string& _out, const Slot &_slot, SerializeFlags _flags = SerializeFlag_NONE )const;
        std::string& serialize_output(std::string& _out, const Slot &_slot, SerializeFlags flags = SerializeFlag_NONE )const;
        std::string& serialize_node(std::string &_out, const Node* node, SerializeFlags _flags = SerializeFlag_NONE ) const;
        std::string& serialize_scope(std::string& _out, const Scope *_scope)const;
        std::string& serialize_for_loop(std::string& _out, const ForLoopNode *_for_loop)const;
        std::string& serialize_while_loop(std::string& _out, const WhileLoopNode *_while_loop_node)const;
        std::string& serialize_cond_struct(std::string& _out, const IfNode*_condition_struct ) const;
        std::string& serialize_variable(std::string& _out, const VariableNode*) const;
        std::string& serialize_property(std::string &_out, const Property*) const;

        // Language definition -------------------------------------------------------------------------

    public:
        invokable_ptr         find_function( const char* _signature ) const;           // Find a function by signature as string (ex:   "int multiply(int,int)" )
        invokable_ptr         find_function(const tools::func_type*) const;               // Find a function by signature (strict first, then cast allowed)
        invokable_ptr         find_function_exact(const tools::func_type*) const;         // Find a function by signature (no cast allowed).
        invokable_ptr         find_function_fallback(const tools::func_type*) const;      // Find a function by signature (casts allowed).
        invokable_ptr         find_operator_fct(const tools::func_type*) const;           // Find an operator's function by signature (strict first, then cast allowed)
        invokable_ptr         find_operator_fct_exact(const tools::func_type*) const;     // Find an operator's function by signature (no cast allowed).
        invokable_ptr         find_operator_fct_fallback(const tools::func_type*) const;  // Find an operator's function by signature (casts allowed).
        const tools::Operator*   find_operator(const std::string& , tools::Operator_t) const;// Find an operator by symbol and type (unary, binary or ternary).
        const Invokable_vec&  get_api()const { return m_functions; }                      // Get all the functions registered in the language. (TODO: why do we store the declared functions here? can't we load them in the VirtualMachine instead?).
        std::string&          to_string(std::string& /*out*/, const tools::type*)const;   // Convert a type to string (by ref).
        std::string&          to_string(std::string& /*out*/, Token_t)const;              // Convert a type to a token_t (by ref).
        std::string           to_string(const tools::type *) const;                       // Convert a type to string.
        std::string           to_string(Token_t)const;                                    // Convert a type to a token_t.
        const tools::type*    get_type(Token_t _token)const;                              // Get the type corresponding to a given token_t (must be a type keyword)
        void                  add_function(std::shared_ptr<const tools::IInvokable>);     // Adds a new function (regular or operator's implementation).
        int                   get_precedence(const tools::IInvokable*)const;              // Get the precedence of a given function (precedence may vary because function could be an operator implementation).

        template<typename T> void load_library(); // Instantiate a library from its type (uses reflection to get all its static methods).
    private:
        invokable_ptr         find_function(u32_t _hash) const;
    private:
        struct {
            std::vector<std::tuple<const char*, Token_t>>                  keywords;
            std::vector<std::tuple<const char*, Token_t, const tools::type*>> types;
            std::vector<std::tuple<const char*, tools::Operator_t, int>>      operators;
            std::vector<std::tuple<char, Token_t>>                         chars;
        } m_definition; // language definition

        operators_vec                                     m_operators;                // the allowed operators (!= implementations).
        Invokable_vec                                     m_operators_impl;           // operators' implementations.
        Invokable_vec                                     m_functions;                // all the functions (including operator's).
        std::unordered_map<u32_t , std::shared_ptr<const tools::IInvokable>> m_functions_by_signature; // Functions indexed by signature hash
        std::unordered_map<Token_t, char>                 m_single_char_by_keyword;
        std::unordered_map<Token_t, const char*>          m_keyword_by_token_t;       // token_t to string (ex: Token_t::keyword_double => "double").
        std::unordered_map<tools::type::id_t, const char*>m_keyword_by_type_id;
        std::unordered_map<char, Token_t>                 m_token_t_by_single_char;
        std::unordered_map<size_t, Token_t>               m_token_t_by_keyword;       // keyword reserved by the language (ex: int, string, operator, if, for, etc.)
        std::unordered_map<tools::type::id_t, Token_t>    m_token_t_by_type_id;
        std::unordered_map<Token_t, const tools::type*>   m_type_by_token_t;          // token_t to type. Works only if token_t refers to a type keyword.

    };

    template<typename T>
    void Nodlang::load_library()
    {
        T library; // Libraries are static and this will force static code to run. TODO: add load/release methods

        auto type = tools::type::get<T>();
        for(auto& each_static : type->get_static_methods())
        {
            add_function(each_static);
        }
    }

    Nodlang* init_language();
    Nodlang* get_language();
    void shutdown_language(Nodlang *pNodlang);
}

