#pragma once

#include <vector>

#include "bdc/String.hpp"
#include "bdc/String_Builder.hpp"
#include "bdc/Array.hpp"

#include "core/reflection/Operator.h"

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

    //
    // This struct holds the definition of the main Nodable's Language.
    // Currently the language is not super evolved, but it matches some basics from C/C++
    //
	struct Language
    {
        bool                    strict_mode;    // When strict mode is ON, any use of undeclared symbol is rejected.
                                                // When OFF, parser can produce a graph with undeclared symbols but the compiler won't be able to handle it.
        bdc::String             buffer;         // TODO: rename, this is the input_buffer, for parsing only.
        Token_Ribbon            ribbon;         // TODO: rename, this is the ribbon state, for parsing only.
        std::vector<Node_Slot*> flow_out;       // TODO: rename, this is the last flow out slot known, for parsing only.
        Graph*                  graph;          // TODO: rename, not owned in/out Graph.

        // data used to initialize all the indexes

        struct {
            std::vector<std::tuple<bdc::String, Token_Type>>                                keywords;
            std::vector<std::tuple<bdc::String, Token_Type, const tools::Type_Descriptor*>> types;
            std::vector<tools::Operator>                                                    operators;
            std::vector<std::tuple<char, Token_Type>>                                       chars;
        } definition; 

        // indexes

        std::vector<tools::Operator>                                    operators;                      // the allowed operators, not their implementations or signature.
        std::unordered_map<Token_Type, char>                            single_char_by_keyword;
        std::unordered_map<Token_Type, const bdc::String>               keyword_by_token_type;          // ex: Token_t::keyword_double => "double".
        std::unordered_map<size_t, Token_Type>                          token_type_by_keyword;          // opposite of keyword_by_token_type
        std::unordered_map<std::type_index, const bdc::String>          keyword_by_type_id;
        std::unordered_map<std::type_index, Token_Type>                 token_type_by_type_id;
        std::unordered_map<char, Token_Type>                            token_type_by_single_char;
        std::unordered_map<Token_Type, const tools::Type_Descriptor*>   type_descriptor_by_token_type;  // some Token_Type are associated with a Type_Descriptor (ex: Token_Type_LITERAL_STRING)
    };

    // Text to Graph ----------------------------------------------------------------------

    Token                           lang_parse_token(const Language&, bdc::String&); // parse a single token from position _cursor in _string.
    bool                            lang_parse_bool_or(const Language&, bdc::String&, bool default_value );
    double                          lang_parse_double_or(const Language&, bdc::String&, double default_value );
    int                             lang_parse_int_or(const Language&, bdc::String&, int default_value );

    void                            lang_reset(Language&, Graph*, bdc::String buffer = "");
    bool                            lang_parse(Language&, Graph* /* graph (out) */, bdc::String /* code (in) */); // Try to convert a source code (input string) to a program tree (output graph). Return true if evaluation went well and false otherwise.
    Scope*                          lang_parse_program(Language&);
    Node*                           lang_parse_code_block(Language&, Scope* parent_scope, Node_Slot* flow_out);
    Node*                           lang_parse_atomic_code_block(Language&, Scope* parent_scope, Node_Slot* flow_out);
    Node*                           lang_parse_scoped_block(Language&, Scope* parent_scope, Node_Slot* flow_out);
    Node*                           lang_parse_expression_block(Language&, Scope* parent_scope, Node_Slot* flow_out, Node_Slot* value_in = nullptr);
    Node*                           lang_parse_if_block(Language&, Scope* parent_scope, Node_Slot* flow_out);
    Node*                           lang_parse_for_block(Language&, Scope* parent_scope, Node_Slot* flow_out);
    Node*                           lang_parse_while_block(Language&, Scope* parent_scope, Node_Slot* flow_out);
    Node*                           lang_parse_empty_block(Language&, Scope* parent_scope, Node_Slot* flow_out);
    Node*                           lang_parse_return(Language&, Scope* parent_scope, Node_Slot* flow_out);
    Node_Slot*                      lang_parse_variable_declaration(Language&, Scope* parent_scope);
    Node_Slot*                      lang_parse_function_call(Language&, Scope* parent_scope);
    Node_Slot*                      lang_parse_parenthesis_expression(Language&, Scope* parent_scope);
    Node_Slot*                      lang_parse_unary_operator_expression(Language&, Language&, Scope* parent_scope, u8_t _precedence = 0);
    Node_Slot*                      lang_parse_binary_operator_expression(Language&, Scope* parent_scope, u8_t _precedence, Node_Slot* _left);
    Node_Slot*                      lang_parse_atomic_expression(Language&, Scope* parent_scope);
    Node_Slot*                      lang_parse_expression(Language&, Scope* parent_scope, u8_t _precedence = 0, Node_Slot* _left_override = nullptr);
    Node_Slot*                      lang_token_to_slot(const Language&, Scope* parent_scope, const Token& _token);
    bool                            lang_tokenize(Language&); // tokenise from current parser state
    bool                            lang_tokenize(Language&, const bdc::String&); // Tokenize a string, return true for success. Tokens are stored in the token ribbon.

    // Graph to Text ------------------------------------------------------------------

    [[nodiscard]] bdc::String       lang_serialize_bool(const Language&, bool b);
    [[nodiscard]] bdc::String       lang_serialize_int(const Language&, int i);
    [[nodiscard]] bdc::String       lang_serialize_double(const Language&, double d);
    [[nodiscard]] bdc::String       lang_serialize_token_type_default(const Language&,Token_Type _token_t);
    [[nodiscard]] bdc::String       lang_serialize_token(const Language&, const Token &);
    [[nodiscard]] bdc::String       lang_serialize_type(const Language&, const tools::Type_Descriptor *_type);
    
    bdc::String_Builder&            lang_serialize_graph(const Language&, bdc::String_Builder& out, const Graph* in);
    const Node_Slot*                lang_serialize_invokable(const Language&, bdc::String_Builder& out, const Node*);
    bdc::String_Builder&            lang_serialize_invokable_sig(const Language&, bdc::String_Builder& out, const tools::IInvokable*);
    bdc::String_Builder&            lang_serialize_func_call(const Language&, bdc::String_Builder& out, const tools::Function_Descriptor *_signature, const bdc::Array<Node_Slot*>& inputs);
    bdc::String_Builder&            lang_serialize_func_sig(const Language&, bdc::String_Builder& out, const tools::Function_Descriptor*);
    bdc::String_Builder&            lang_serialize_input(const Language&, bdc::String_Builder& out, const Node_Slot *_slot, Serialization_Flags _flags = Serialization_Flag_NONE );
    bdc::String_Builder&            lang_serialize_value_out(const Language&, bdc::String_Builder& out, const Node_Slot *slot, Serialization_Flags _flags = Serialization_Flag_NONE );
    bdc::String_Builder&            lang_serialize_node(const Language&, bdc::String_Builder& out, const Node*, Serialization_Flags _flags = Serialization_Flag_NONE);
    bdc::String_Builder&            lang_serialize_scope(const Language&, bdc::String_Builder& out, const Scope*);
    bdc::String_Builder&            lang_serialize_for_loop(const Language&, bdc::String_Builder& out, const Node* _for_loop);
    bdc::String_Builder&            lang_serialize_while_loop(const Language&, bdc::String_Builder& out, const Node*_while_loop_node);
    bdc::String_Builder&            lang_serialize_cond_struct(const Language&, bdc::String_Builder& out, const Node* if_node );
    bdc::String_Builder&            lang_serialize_literal(const Language&, bdc::String_Builder& out, const Node*);
    bdc::String_Builder&            lang_serialize_variable(const Language&, bdc::String_Builder& out, const Node*);
    bdc::String_Builder&            lang_serialize_variable_ref(const Language&, bdc::String_Builder& out, const Node *_node);
    bdc::String_Builder&            lang_serialize_empty_instruction(const Language&, bdc::String_Builder& out, const Node *_node);
    bdc::String_Builder&            lang_serialize_property(const Language&, bdc::String_Builder& out, const Node_Property*);
    bdc::String_Builder&            lang_serialize_return(const Language&, bdc::String_Builder& out, const Node*);

    // General read-only procedures -------------------------------------------------------------------------

    bool                            lang_is_operator(const Language&, const tools::Function_Descriptor*);
    const tools::Operator*          lang_find_operator(const Language&,  const tools::Operator& op); // op.precedence is ignored in operator== for tools::Operator
    Token_Type                      lang_to_literal_token(const Language&, const tools::Type_Descriptor*);
    int                             lang_get_precedence(const Language&, const tools::Function_Descriptor*);         // Get the precedence of a given function (precedence may vary because function could be an operator implementation).
    const tools::Type_Descriptor*   lang_get_type(const Language&, Token_Type _token);                               // Get the type corresponding to a given token_t (must be a type keyword)

    // Language management -----------------------------------------------------------------------------------

    Language&                       language_init();
    void                            language_shutdown(); // undo init_language()
    [[deprecated]] bool             language_is_initialized(); // TODO: this should not exist, user must know if he already initialized a language. The problem comes from the fact some functions in this struct are used by code that do not need a language.
    Language&                       language();
}

