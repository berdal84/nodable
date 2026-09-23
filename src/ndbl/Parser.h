#pragma once

#include "bdc/String.hpp"
#include "bdc/String_Builder.hpp"
#include "bdc/Array.hpp"

#include "reflection/Operator.h"
#include "Token.h"
#include "Token_Ribbon.h"
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
        const Language_Definition*          langdef;
        bool                                strict_mode;    // When strict mode is ON, any use of undeclared symbol is rejected.
                                                            // When OFF, parser can produce a graph with undeclared symbols but the compiler won't be able to handle it.
        bdc::String                         buffer;
        Token_Ribbon                        ribbon;
        Graph*                              graph;
        bdc::Resizable_Array<Node_Slot*>    flow_out; // TODO: rename, this is the last flow out slot known, for parsing only.
        bdc::String_Builder                 sb;
    };

    void                            parser_init(Parser_Context& parser);
    void                            parser_deinit(Parser_Context& parser);
    void                            parser_reset(Parser_Context&, Graph*, bdc::String buffer = "");
    String                          parser_build_tstring(Parser_Context&);
    String                          parser_build_string(Parser_Context&);

    bool                            tokenize(Parser_Context&); // tokenise from current parser state
    bool                            tokenize(Parser_Context&, const bdc::String&); // Tokenize a string, return true for success. Tokens are stored in the token ribbon.

    Token                           parse_token(const Parser_Context&, bdc::String&); // parse a single token from position _cursor in _string.
    bool                            parse_bool_or(const Parser_Context&, bdc::String&, bool default_value );
    double                          parse_double_or(const Parser_Context&, bdc::String&, double default_value );
    int                             parse_int_or(const Parser_Context&, bdc::String&, int default_value );
    
    bool                            parse(Parser_Context&, Graph* /* graph (out) */, bdc::String /* code (in) */); // Try to convert a source code (input string) to a program tree (output graph). Return true if evaluation went well and false otherwise.
    Scope*                          parse_program(Parser_Context&);
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

    bdc::String                     serialize_bool(const Parser_Context&, bool b);
    bdc::String                     serialize_int(const Parser_Context&, int i);
    bdc::String                     serialize_double(const Parser_Context&, double d);
    bdc::String                     serialize_type(const Parser_Context&, const Type_Descriptor *_type);
    bdc::String                     serialize_token_type(const Parser_Context&, Token_Type _token_t);
    bdc::String                     serialize_token(const Parser_Context&, const Token&);
    void                            serialize_graph(Parser_Context&, const Graph* in);
    void                            serialize_function_call(Parser_Context&, const Type_Descriptor *_signature, const bdc::Array<Node_Slot*>& inputs);
    void                            serialize_function_type(Parser_Context&, const Type_Descriptor*);
    void                            serialize_input(Parser_Context&, const Node_Slot *_slot, Serialization_Flags _flags = Serialization_Flag_NONE );
    void                            serialize_node(Parser_Context&, const Node*, Serialization_Flags _flags = Serialization_Flag_NONE);
    void                            serialize_node_value_out(Parser_Context&, const Node_Slot *slot, Serialization_Flags _flags = Serialization_Flag_NONE );
    void                            serialize_scope(Parser_Context&, const Scope*);
    void                            serialize_for_loop(Parser_Context&, const Node* _for_loop);
    void                            serialize_while_loop(Parser_Context&, const Node*_while_loop_node);
    void                            serialize_if_else(Parser_Context&, const Node* if_node );
    void                            serialize_literal(Parser_Context&, const Node*);
    void                            serialize_variable(Parser_Context&, const Node*);
    void                            serialize_variable_ref(Parser_Context&, const Node *_node);
    void                            serialize_empty_instruction(Parser_Context&, const Node *_node);
    void                            serialize_property(Parser_Context&, const Node_Property*);
    void                            serialize_return(Parser_Context&, const Node*);
}

