#pragma once

#include <stack>   // TODO: use bdc::Resizable_Array
#include <vector>  // TODO: use bdc::Resizable_Array

#include "bdc/String.hpp"
#include "bdc/String_Builder.hpp"
#include "bdc/Array.hpp"

#include "reflection/Operator.h"
#include "Token.h"
#include "Graph.h"

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

    struct Keyword
    {
        bdc::String id;
        Token_Type  token_type;
    };

    struct Type
    {
        bdc::String             id; // identifier / keyword (ex: "int", "double", "String")
        Token_Type              token_type;
        const Type_Descriptor*  type_descriptor;
    };

    struct Character
    {
        char       id;
        Token_Type token_type;
    };

    //
    // This struct holds the definition of the main Nodable's Parser.
    // Currently the parser is not super evolved, but it matches some basics from C/C++
    //
	struct Language_Definition
    {
        // definitions
        bdc::Resizable_Array<Keyword>   keywords;
        bdc::Resizable_Array<Type>      types;
        bdc::Resizable_Array<Operator>  operators;
        bdc::Resizable_Array<Character> chars;

        // indexes
        bdc::Hash_Map<Token_Type, char>                     single_char_by_keyword;
        bdc::Hash_Map<Token_Type, bdc::String>              keyword_by_token_type;          // ex: Token_t::keyword_double => "double".
        bdc::Hash_Map<size_t, Token_Type>                   token_type_by_keyword;          // opposite of keyword_by_token_type
        bdc::Hash_Map<size_t, bdc::String>                  keyword_by_type_id;
        bdc::Hash_Map<size_t, Token_Type>                   token_type_by_type_id;
        bdc::Hash_Map<char, Token_Type>                     token_type_by_single_char;
        bdc::Hash_Map<Token_Type, const Type_Descriptor*>   type_descriptor_by_token_type;  // some Token_Type are associated with a Type_Descriptor (ex: Token_Type_LITERAL_STRING)
    };
    
    Language_Definition&            langdef_init();
    void                            langdef_shutdown(); // undo init_language()
    [[deprecated]] bool             langdef_is_initialized(); // TODO: this should not exist, user must know if he already initialized a parser. The problem comes from the fact some functions in this struct are used by code that do not need a parser.
    Language_Definition&            langdef();
    bool                            langdef_is_operator(const Language_Definition&, const Type_Descriptor*);
    const Operator*                 langdef_find_operator(const Language_Definition&,  const Operator& op); // op.precedence is ignored in operator== for Operator
    int                             langdef_get_precedence(const Language_Definition&, const Type_Descriptor*);         // Get the precedence of a given function (precedence may vary because function could be an operator implementation).
    const Type_Descriptor*          langdef_get_type_descriptor_from_token_type(const Language_Definition&, Token_Type _token);                               // Get the type corresponding to a given token_t (must be a type keyword)
    Token_Type                      langdef_type_descriptor_to_token_type_literal(const Language_Definition&, const Type_Descriptor*);
 
	struct Parser_Context
    {
        bool                                success;
        const Language_Definition*          langdef;
        bool                                strict_mode;
        Graph*                              out_graph;
        bdc::String                         in_text;
        bdc::Resizable_Array<Node_Slot*>    flow_out;
        size_t                              cursor; // current token index
        Token                               global_token; // for any prefix/suffix that can't be attached to a Token.
        std::vector<Token>                  tokens; // contains the strict minimum (extra tokens are prefix/suffixes)
        std::vector<Token>                  pristine_tokens; // includes spaces, line returns, etc.
        std::stack<size_t>                  transaction; // transaction start indexes
    };


    void                            parser_init(Parser_Context&, Graph* /* out */ = nullptr, bdc::String /* in */ = "");
    void                            parser_deinit(Parser_Context&);
    void                            parser_reset(Parser_Context&, Graph* /* out */, bdc::String /* in */);
    bool                            parser_tokenize(Parser_Context&);
    bool                            parser_can_eat(const Parser_Context&, size_t token_count = 1);
    Token                           parser_eat(Parser_Context&); // Return the next token and increment cursor
    Token                           parser_eat_if(Parser_Context&, Token_Type); // Only if next token has a given type: returns it and increment cursor
    const Token&                    parser_get_eaten(const Parser_Context&);
    bool                            parser_peek(const Parser_Context&, Token_Type);
    const Token&                    parser_peek(const Parser_Context&);
    Token&                          parser_push(Parser_Context&, Token&);
    bdc::String                     parser_to_string(const Parser_Context&, size_t begin, size_t end) ; // Format ribbon from range [begin, end-1]
    bdc::String                     parser_to_string(const Parser_Context&); // Generate a colored string highlighting the current and past tokens
    void                            parser_start_transaction(Parser_Context&);    // Start a transaction by saving the cursor position in a stack (allows nested transactions).
    void                            parser_rollback(Parser_Context&); // Restore the cursor position where the last transaction started.
    void                            parser_commit(Parser_Context&);   // Commit the current transaction.

    bool                            parse_graph(Parser_Context&);
    Node*                           parse_code_block(Parser_Context&, Scope* parent_scope, Node_Slot* flow_out);
    Node*                           parse_atomic_code_block(Parser_Context&, Scope* parent_scope, Node_Slot* flow_out);
    Node*                           parse_scoped_block(Parser_Context&, Scope* parent_scope, Node_Slot* flow_out);
    Node*                           parse_expression_block(Parser_Context&, Scope* parent_scope, Node_Slot* flow_out, Node_Slot* value_in = nullptr);
    Node*                           parse_if_block(Parser_Context&, Scope* parent_scope, Node_Slot* flow_out);
    Node*                           parse_for_block(Parser_Context&, Scope* parent_scope, Node_Slot* flow_out);
    Node*                           parse_while_block(Parser_Context&, Scope* parent_scope, Node_Slot* flow_out);
    Node*                           parse_empty_block(Parser_Context&, Scope* parent_scope, Node_Slot* flow_out);
    Node*                           parse_return(Parser_Context&, Scope* parent_scope, Node_Slot* flow_out);
    Node_Slot*                      parse_variable_declaration(Parser_Context&, Scope* parent_scope);
    Node_Slot*                      parse_function_call(Parser_Context&, Scope* parent_scope);
    Node_Slot*                      parse_parenthesis_expression(Parser_Context&, Scope* parent_scope);
    Node_Slot*                      parse_unary_operation_expression(Parser_Context&, Parser_Context&, Scope* parent_scope, u8_t _precedence = 0);
    Node_Slot*                      parse_binary_operation_expression(Parser_Context&, Scope* parent_scope, u8_t _precedence, Node_Slot* _left);
    Node_Slot*                      parse_atomic_expression(Parser_Context&, Scope* parent_scope);
    Node_Slot*                      parse_expression(Parser_Context&, Scope* parent_scope, u8_t _precedence = 0, Node_Slot* _left_override = nullptr);
    Node_Slot*                      parse_token(const Parser_Context&, Scope* parent_scope, const Token& _token);
    Token                           parse_token(const Parser_Context&, bdc::String&); // parse a single token from position _cursor in _string.
    bool                            parse_bool_or(const Parser_Context&, bdc::String&, bool default_value );
    double                          parse_double_or(const Parser_Context&, bdc::String&, double default_value );
    int                             parse_int_or(const Parser_Context&, bdc::String&, int default_value );

    struct Serializer_Context
    {
        bool                                success;
        const Language_Definition*          langdef;
        bool                                strict_mode;
        Graph*                              in_graph;
        bdc::String_Builder                 out_sb;
    };

    void                            serializer_init(Serializer_Context&, Graph* /* in */ = nullptr );
    void                            serializer_deinit(Serializer_Context&);
    void                            serializer_reset(Serializer_Context&, Graph* /* in */ );
    String                          serializer_build_tstring(Serializer_Context&);
    String                          serializer_build_string(Serializer_Context&);

    void                            serialize_graph(Serializer_Context&);
    void                            serialize_function_call(Serializer_Context&, const Type_Descriptor*, const bdc::Array<Node_Slot*>& inputs);
    void                            serialize_function_type(Serializer_Context&, const Type_Descriptor*);
    void                            serialize_input(Serializer_Context&, const Node_Slot *_slot, Serialization_Flags = Serialization_Flag_NONE );
    void                            serialize_node(Serializer_Context&, const Node*, Serialization_Flags = Serialization_Flag_NONE);
    void                            serialize_node_value_out(Serializer_Context&, const Node_Slot*, Serialization_Flags = Serialization_Flag_NONE );
    void                            serialize_scope(Serializer_Context&, const Scope*);
    void                            serialize_for_loop(Serializer_Context&, const Node*);
    void                            serialize_while_loop(Serializer_Context&, const Node*);
    void                            serialize_if_else(Serializer_Context&, const Node*);
    void                            serialize_literal(Serializer_Context&, const Node*);
    void                            serialize_variable(Serializer_Context&, const Node*);
    void                            serialize_variable_ref(Serializer_Context&, const Node*);
    void                            serialize_empty_instruction(Serializer_Context&, const Node*);
    void                            serialize_property(Serializer_Context&, const Node_Property*);
    void                            serialize_return(Serializer_Context&, const Node*);
    bdc::String                     serialize_bool(const Serializer_Context&, bool b);
    bdc::String                     serialize_int(const Serializer_Context&, int i);
    bdc::String                     serialize_double(const Serializer_Context&, double d);
    bdc::String                     serialize_type(const Serializer_Context&, const Type_Descriptor*);
    bdc::String                     serialize_token_type(const Serializer_Context&, Token_Type);
    bdc::String                     serialize_token(const Serializer_Context&, const Token&);
}

