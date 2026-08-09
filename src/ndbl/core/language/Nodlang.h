#pragma once

#include <string>
#include <vector>
#include "core/reflection/Operator_Type.h"
#include "tools/core/Containers.h"

#include "ndbl/core/Token.h"
#include "ndbl/core/Token_Ribbon.h"
#include "ndbl/core/Graph.h"

namespace ndbl
{
    // forward declarations
    class Scope;
    class Node;
    class Node_Property;

    typedef int Serialization_Flags;
    enum Serialization_Flag_
    {
        Serialization_Flag_NONE             = 0,
        Serialization_Flag_RECURSE          = 1 << 0,
        Serialization_Flag_WRAP_WITH_BRACES = 1 << 1
    };

    /**
	 * Nodlang is Nodable's language.
	 * This class define Nodlang language, and provide a parser/serializer.
     * Syntax is pretty close from C/C++
	 */
	class Nodlang
    {
    public:
        explicit Nodlang(bool _strict = false);
		~Nodlang();

        // Parser /////////////////////////////////////////////////////////////////////
        bool                            parse(Graph* graph_out, const std::string& code_in); // Try to convert a source code (input string) to a program tree (output graph). Return true if evaluation went well and false otherwise.
        //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        Scope*                       parse_program();
        Node*                        parse_code_block(Scope* parent_scope, Node_Slot* flow_out);
        Node*                        parse_atomic_code_block(Scope* parent_scope, Node_Slot* flow_out);
        Node*                        parse_scoped_block(Scope* parent_scope, Node_Slot* flow_out);
        Node*                        parse_expression_block(Scope* parent_scope, Node_Slot* flow_out, Node_Slot* value_in = nullptr);
        Node*                        parse_if_block(Scope* parent_scope, Node_Slot* flow_out);
        Node*                        parse_for_block(Scope* parent_scope, Node_Slot* flow_out);
        Node*                        parse_while_block(Scope* parent_scope, Node_Slot* flow_out);
        Node*                        parse_empty_block(Scope* parent_scope, Node_Slot* flow_out);
        Node*                        parse_return(Scope* parent_scope, Node_Slot* flow_out);
        //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        Node_Slot*                    parse_variable_declaration(Scope* parent_scope);
        Node_Slot*                    parse_function_call(Scope* parent_scope);
        Node_Slot*                    parse_parenthesis_expression(Scope* parent_scope);
        Node_Slot*                    parse_unary_operator_expression(Scope* parent_scope, u8_t _precedence = 0);
        Node_Slot*                    parse_binary_operator_expression(Scope* parent_scope, u8_t _precedence, Node_Slot* _left);
        Node_Slot*                    parse_atomic_expression(Scope* parent_scope);
        Node_Slot*                    parse_expression(Scope* parent_scope, u8_t _precedence = 0, Node_Slot* _left_override = nullptr);
        Node_Slot*                    token_to_slot(Scope* parent_scope, const Token& _token);
        //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        bool                            tokenize(); // tokenise from current parser state
        bool                            tokenize(const std::string& _string); // Tokenize a string, return true for success. Tokens are stored in the token ribbon.
        Token                           parse_token(const std::string& _string) const;
        Token                           parse_token(const char *buffer, size_t buffer_size, size_t &global_cursor) const; // parse a single token from position _cursor in _string.
        bool                            parse_bool_or(const std::string&, bool default_value ) const;
        double                          parse_double_or(const std::string&, double default_value ) const;
        int                             parse_int_or(const std::string&, int default_value ) const;
        std::string                     remove_quotes(const std::string& _quoted_str) const;

    private:
        bool                            accepts_suffix(Token_Type type) const;
		bool                            is_syntax_valid(); // Check if the syntax of the token ribbon is correct. (ex: ["12", "-"] is incorrect)
    public:
        struct ParserState
        {
            void                reset(Graph* g) { reset_ribbon(); reset_graph(g); }
            void                reset_ribbon(const char* new_buf = nullptr, size_t new_size = 0);
            void                reset_graph(Graph*);
            const char*         buffer() const { ASSERT(_buffer.data); return _buffer.data; }
            size_t              buffer_size() const { return _buffer.size; }
            std::string         string() const { return _ribbon.to_string(); }; // Ribbon's
            Graph*              graph() const { ASSERT(_graph); return _graph; }
            Token_Ribbon&       tokens()  { return _ribbon; }
            const char*         buffer_at(size_t offset) const { ASSERT(offset < _buffer.size ); return _buffer.data + offset; }
            void                start_transaction() { _ribbon.start_transaction(); }
            void                commit() { _ribbon.commit(); }
            void                rollback() { _ribbon.rollback(); }

        private:
            struct Buffer
            {
                const char* data = nullptr; // NOT owned
                size_t      size = 0;
            };

            Buffer                  _buffer;
            Graph*                  _graph = nullptr; // NOT owned
            Token_Ribbon            _ribbon;
            std::vector<Node_Slot*> _flow_out; // last flow out slot known
        } _state;

    private: bool m_strict_mode; // When strict mode is ON, any use of undeclared symbol is rejected.
                                 // When OFF, parser can produce a graph with undeclared symbols but the compiler won't be able to handle it.

        // Serializer ------------------------------------------------------------------
    public:
        std::string& serialize_graph(std::string& _out, const Graph* graph ) const;
        std::string& serialize_bool(std::string& _out, bool b) const;
        std::string& serialize_int(std::string& _out, int i) const;
        std::string& serialize_double(std::string& _out, double d) const;
        const Node_Slot*  serialize_invokable(std::string&_out, const Node*) const;
        std::string& serialize_invokable_sig(std::string& _out, const tools::IInvokable*)const;
        std::string& serialize_func_call(std::string& _out, const tools::Function_Descriptor *_signature, tools::Array_View<const Node_Slot*> inputs)const;
        std::string& serialize_func_sig(std::string& _out, const tools::Function_Descriptor*)const;
        std::string& serialize_default_buffer(std::string& _out, Token_Type _token_t)const;
        std::string& serialize_token(std::string& _out, const Token &) const;
        std::string& serialize_type(std::string& _out, const tools::Type_Descriptor*) const;
        std::string  serialize_type(const tools::Type_Descriptor *_type) const;
        std::string& serialize_input(std::string& _out, const Node_Slot *_slot, Serialization_Flags _flags = Serialization_Flag_NONE )const;
        std::string& serialize_value_out(std::string& _out, const Node_Slot *slot, Serialization_Flags _flags = Serialization_Flag_NONE )const;
        std::string& serialize_node(std::string &_out, const Node*, Serialization_Flags _flags = Serialization_Flag_NONE) const;
        std::string& serialize_scope(std::string& _out, const Scope*)const;
        std::string& serialize_for_loop(std::string& _out, const Node* _for_loop)const;
        std::string& serialize_while_loop(std::string& _out, const Node*_while_loop_node)const;
        std::string& serialize_cond_struct(std::string& _out, const Node* if_node ) const;
        std::string& serialize_literal(std::string& _out, const Node*) const;
        std::string& serialize_variable(std::string& _out, const Node*) const;
        std::string& serialize_variable_ref(std::string &_out, const Node *_node) const;
        std::string& serialize_empty_instruction(std::string &_out, const Node *_node) const;
        std::string& serialize_property(std::string &_out, const Node_Property*) const;
        std::string& serialize_return(std::string& out, const Node*) const;

        // Language definition -------------------------------------------------------------------------

    public:
        bool                   is_operator(const tools::Function_Descriptor*) const;
        const tools::Operator* find_operator(const std::string& , tools::Operator_Type) const;// Find an operator by symbol and type (unary, binary or ternary).
        Token_Type             to_literal_token(const tools::Type_Descriptor*) const;
        int                    get_precedence(const tools::Function_Descriptor*)const;                // Get the precedence of a given function (precedence may vary because function could be an operator implementation).
        const tools::Type_Descriptor* get_type(Token_Type _token)const;                              // Get the type corresponding to a given token_t (must be a type keyword)
    private:
        struct {
            std::vector<std::tuple<const char*, Token_Type>>                  keywords;
            std::vector<std::tuple<const char*, Token_Type, const tools::Type_Descriptor*>> types;
            std::vector<std::tuple<const char*, tools::Operator_Type, int>>      operators;
            std::vector<std::tuple<char, Token_Type>>                         chars;
        } m_definition; // language definition

        std::vector<const tools::Operator*>               m_operators;                // the allowed operators (!= implementations).
        std::unordered_map<Token_Type, char>                 m_single_char_by_keyword;
        std::unordered_map<Token_Type, const char*>          m_keyword_by_token_t;       // token_t to string (ex: Token_t::keyword_double => "double").
        std::unordered_map<std::type_index, const char*>  m_keyword_by_type_id;
        std::unordered_map<char, Token_Type>                 m_token_t_by_single_char;
        std::unordered_map<size_t, Token_Type>               m_token_t_by_keyword;       // keyword reserved by the language (ex: int, string, operator, if, for, etc.)
        std::unordered_map<std::type_index, Token_Type>      m_token_t_by_type_id;
        std::unordered_map<Token_Type, const tools::Type_Descriptor*>   m_type_by_token_t;          // token_t to type. Works only if token_t refers to a type keyword.
    };

    [[nodiscard]]
    Nodlang*    init_language();
    bool        has_language();
    Nodlang*    get_language();
    void        shutdown_language(Nodlang*); // undo init_language()
}

