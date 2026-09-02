//---------------------------------------------------------------------------------------------------------------------------
// Nodlang.cpp
// This file is structured in 3 parts, use Ctrl + F to search:
//  [SECTION] A. Declaration (types, keywords, etc.)
//  [SECTION] B. Parser
//  [SECTION] C. Serializer
//---------------------------------------------------------------------------------------------------------------------------

#include "Nodlang.h"

#include <algorithm>
#include <cstddef>
#include <limits>
#include "bdc/String.hpp"
#include <cctype> // isdigit, isalpha, and isalnum.

#include "bdc/String_Builder.hpp"
#include "core/Asserts.h"
#include "core/Constants.h"
#include "core/Node_Slot.h"
#include "core/Token_Type.h"
#include "bdc/Types.hpp"
#include "core/reflection/Operator.h"
#include "tools/core/Format.h"
#include "tools/core/Log.h"
#include "tools/core/Hash.h"

#include "ndbl/core/Node.h"
#include "ndbl/core/Node_Property.h"
#include "ndbl/core/Graph.h"
#include "ndbl/core/Scope.h"

// private
namespace ndbl
{
    void            _lang_reset_graph(Language&, Graph*);
    bdc::String     _lang_to_string(const Language&);
    Graph*          _lang_graph(const Language&);
    bdc::String     _lang_rsplit_buffer(const Language&, size_t offset);
    bool            _lang_accepts_suffix(const Language&, Token_Type);
    bool            _lang_is_syntax_valid(const Language&); // Check if the syntax of the token ribbon is correct. (ex: ["12", "-"] is incorrect)

}

namespace ndbl
{
    using namespace bdc;
    using namespace tools;

    static Language* g_language = nullptr;

    Language& language_init()
    {
        ASSERT(g_language == nullptr);

        Language* language = bdc::memory_new<Language>();

        // A.1. Define the language
        //-------------------------
        language->definition.chars =
        {
            { '(',  Token_Type_parenthesis_open},
            { ')',  Token_Type_parenthesis_close},
            { '{',  Token_Type_scope_begin},
            { '}',  Token_Type_scope_end},
            { '\n', Token_Type_ignore},
            { '\t', Token_Type_ignore},
            { ' ',  Token_Type_ignore},
            { ';',  Token_Type_end_of_instruction},
            { ',',  Token_Type_list_separator}
        };

        language->definition.keywords =
        {
            { "if",       Token_Type_keyword_if },
            { "for",      Token_Type_keyword_for },
            { "while",    Token_Type_keyword_while },
            { "else",     Token_Type_keyword_else },
            { "true",     Token_Type_literal_bool },
            { "false",    Token_Type_literal_bool },
            { "operator", Token_Type_keyword_operator },
            { "return",   Token_Type_keyword_return }
        };

        language->definition.types =
        {
            // TODO: instead of using type_get<T>(), I should use a more datadriven option,
            //       I should be able to do type_get(Token_Type_keyword_bool) for example,
            //       Or with an indirection level  type_get( token_type_keyword_to_type(Token_Type_keyword_bool) )  
            { "bool",   Token_Type_keyword_bool,   type_get<bool>()},
            { "string", Token_Type_keyword_string, type_get<bdc::String>()},
            { "double", Token_Type_keyword_double, type_get<double>()},
            { "i16",    Token_Type_keyword_i16,    type_get<i16_t>()},
            { "int",    Token_Type_keyword_int,    type_get<i32_t>()},
            { "any",    Token_Type_keyword_any,    type_get<any>()},
            // we don't really want to parse/serialize that
            // { "unknown",Token_t::keyword_unknown,type_get<unknown>()},
        };

        language->definition.operators =
        {
            {"-",   Operator_Type::Unary,   5},
            {"!",   Operator_Type::Unary,   5},
            {"/",   Operator_Type::Binary, 20},
            {"*",   Operator_Type::Binary, 20},
            {"+",   Operator_Type::Binary, 10},
            {"-",   Operator_Type::Binary, 10},
            {"||",  Operator_Type::Binary, 10},
            {"&&",  Operator_Type::Binary, 10},
            {">=",  Operator_Type::Binary, 10},
            {"<=",  Operator_Type::Binary, 10},
            {"=>",  Operator_Type::Binary, 10},
            {"==",  Operator_Type::Binary, 10},
            {"<=>", Operator_Type::Binary, 10},
            {"!=",  Operator_Type::Binary, 10},
            {">",   Operator_Type::Binary, 10},
            {"<",   Operator_Type::Binary, 10},
            {"=",   Operator_Type::Binary,  0},
            {"+=",  Operator_Type::Binary,  0},
            {"-=",  Operator_Type::Binary,  0},
            {"/=",  Operator_Type::Binary,  0},
            {"*=",  Operator_Type::Binary,  0}
        };

        // A.2. Create indexes
        //---------------------
        for( auto [_char, token_t] : language->definition.chars)
        {
            language->token_type_by_single_char.insert({_char, token_t});
            language->single_char_by_keyword.insert({token_t, _char});
        }

        for( auto [keyword, token_t] : language->definition.keywords)
        {
            language->token_type_by_keyword.insert({string_hash(keyword).hash, token_t});
            language->keyword_by_token_type.insert({token_t, keyword});
        }

        for( auto [keyword, token_t, type] : language->definition.types)
        {
            language->keyword_by_token_type.insert({token_t, keyword});
            language->keyword_by_type_id.insert({type->id, keyword});
            language->token_type_by_keyword.insert({string_hash(keyword).hash, token_t});
            language->token_type_by_type_id.insert({type->id, token_t});
            language->type_descriptor_by_token_type.insert({token_t, type});
        }

        for( const Operator& op : language->definition.operators)
        {
            for(const auto& existing_op : language->operators)
            {
                VERIFY(existing_op != op, "The same operator already exists!");
            }
            language->operators.emplace_back(op);
        }

        g_language = language;

        return *language;
    }

    bool language_is_initialized()
    {
        return g_language != nullptr;
    }

    Language& language()
    {
        VERIFY(g_language, "No language found, did you call init_language?");
        return *g_language;
    }

    void language_shutdown()
    {
        ASSERT(g_language != nullptr);
        bdc::memory_delete(g_language);
        g_language = nullptr;
    }


    //---------------------------------------------------------------------------------------------------------------------------
    // [SECTION] B. Parser ------------------------------------------------------------------------------------------------------
    //---------------------------------------------------------------------------------------------------------------------------

    bdc::String _lang_to_string(const Language& lang)
    {
        return lang.ribbon.to_string();
    };

    Graph* _lang_graph(const Language& lang)
    {
        ASSERT(lang.graph);
        return lang.graph;
    }

    bdc::String _lang_rsplit_buffer(const Language& lang, size_t offset)
    {
        bdc::String result = lang.buffer;
        string_rsplit(result, offset);
        return result;
    }

    void lang_reset(Language& lang, Graph* graph, String buffer)
    {
        lang.buffer = buffer;
        lang.ribbon.reset( buffer );
        lang.graph = graph;
    }

    bool lang_parse(Language& lang, Graph* graph_out, bdc::String code)
    {
        lang_reset(lang, graph_out, code);

        TOOLS_DEBUG_LOG(Verbosity_Diagnostic, "Parser", "Parsing ...\n%s\n", code.c_str() );

        if ( !lang_tokenize(lang, code) )
        {
            return false;
        }

        if ( !_lang_is_syntax_valid(lang) )
        {
            return false;
        }

        Scope* scope = lang_parse_program(lang);

        if ( scope_is_empty(scope) )
        {
            return false;
        }

        if ( lang.ribbon.can_eat() )
        {
            TOOLS_DEBUG_LOG(Verbosity_Diagnostic, "Parser", TOOLS_KO " End of token ribbon expected\n");
            TOOLS_DEBUG_LOG(Verbosity_Diagnostic, "Parser", "%s", Format::title("Token_Ribbon").c_str());
            for (const Token& each_token : lang.ribbon )
            {
                TOOLS_DEBUG_LOG(Verbosity_Diagnostic, "Parser", "token idx %i: %s\n", each_token.index, each_token.json().c_str());
            }
            TOOLS_DEBUG_LOG(Verbosity_Diagnostic, "Parser", "%s", Format::title("Token_Ribbon end").c_str());
            auto curr_token = lang.ribbon.peek();
            TOOLS_DEBUG_LOG(Verbosity_Diagnostic, "Parser", TOOLS_KO " Failed to parse from token %llu/%llu and above.\n", curr_token.index, lang.ribbon.size());
            TOOLS_LOG(Verbosity_Error, "Parser", "Unable to parse all the tokens\n");
            return false;
        }
        return true;
    }

    bool lang_parse_bool_or(const Language& lang, bdc::String& buffer, bool default_value)
    {
        Token token = lang_parse_token( lang, buffer);
        if (token.type == Token_Type_literal_bool )
            return token.word_view() == "true";
        return default_value;
    }

    double lang_parse_double_or(const Language& lang, bdc::String& buffer, double default_value)
    {
        Token token  = lang_parse_token( lang, buffer);

        if (token.type == Token_Type_literal_double )
        {
            return std::stod( token.word_view().c_str() );
        }

        return default_value;
    }

    int lang_parse_int_or(const Language& lang, bdc::String& buffer, int default_value)
    {
        Token token  = lang_parse_token( lang, buffer);

        if (token.type == Token_Type_literal_int )
        {
            i64_t l = atoll(buffer.c_str());
            int n = std::clamp(l, (i64_t)std::numeric_limits<int>::min() , (i64_t)std::numeric_limits<int>::max());
            if( n > (int)l )
            {
                TOOLS_LOG( Verbosity_Warning, "Nodlang", "Parsing a too large integer for 32bits!\n");
            }
            return n;
        }
        return default_value;
    }

    Node_Slot* lang_token_to_slot(const Language& lang, Scope* parent_scope, const Token& _token)
    {
        if (_token.type == Token_Type_identifier)
        {
            bdc::String identifier = _token.word_view();
            if( Node* existing_node = scope_find_variable(parent_scope, identifier) )
            {
                return existing_node->component.variable.ref_out;
            }

            if ( !lang.strict_mode )
            {
                // Insert a VariableNodeRef with "any" type
                TOOLS_LOG(Verbosity_Warning,  "Parser", "%s is not declared (strict mode), abstract graph can be generated but compilation will fail.\n",
                            _token.word_view().c_str() );
                Node* ref = graph_create_variable_ref( lang.graph, parent_scope );
                ref->value->token = _token;
                return ref->value_out();
            }

            TOOLS_LOG(Verbosity_Error,  "Parser", "%s is not declared (strict mode) \n", _token.word_view().c_str() );
            return nullptr;
        }

        Node* literal = nullptr;

        switch (_token.type)
        {
            case Token_Type_literal_bool:   literal = graph_create_literal<bool>(lang.graph, parent_scope );        break;
            case Token_Type_literal_int:    literal = graph_create_literal<i32_t>( lang.graph, parent_scope );       break;
            case Token_Type_literal_double: literal = graph_create_literal<double>( lang.graph, parent_scope );      break;
            case Token_Type_literal_string: literal = graph_create_literal<bdc::String>( lang.graph, parent_scope ); break;
            default:
                break; // we don't want to throw
        }

        if ( literal )
        {
            TOOLS_DEBUG_LOG(
                Verbosity_Diagnostic, "Parser", TOOLS_OK " Token %s converted to a Literal %s\n",
                _token.word_view().c_str(),
                literal->value->type->name.c_str()
            );
            literal->value->token = _token;
            return literal->value_out();
        }

        TOOLS_DEBUG_LOG(Verbosity_Diagnostic, "Parser", TOOLS_KO " Unable to run token_to_slot with token %s!\n", _token.word_view().c_str());
        return nullptr;
    }

    Node_Slot* lang_parse_binary_operator_expression(Language& lang, Scope* parent_scope, u8_t _precedence, Node_Slot* _left)
    {
        TOOLS_DEBUG_LOG(Verbosity_Diagnostic, "Parser", "Parsing binary expression ...\n");
        ASSERT(_left != nullptr);

        if (!lang.ribbon.can_eat(2))
        {
            TOOLS_DEBUG_LOG(Verbosity_Diagnostic, "Parser", TOOLS_KO " Not enough tokens\n");
            return nullptr;
        }

        lang.ribbon.start_transaction();
        const Token operator_token = lang.ribbon.eat();
        const Token operand_token  = lang.ribbon.peek();

        // Structure check
        const bool isValid = operator_token.type == Token_Type_operator &&
                            operand_token.type != Token_Type_operator;

        if (!isValid)
        {
            lang.ribbon.rollback();
            TOOLS_DEBUG_LOG(Verbosity_Diagnostic, "Parser", TOOLS_KO " Unexpected tokens\n");
            return nullptr;
        }

        const Operator *ope = lang_find_operator(lang, Operator{ operator_token.word_view(), Operator_Type::Binary} );
        if (ope == nullptr)
        {
            TOOLS_DEBUG_LOG(Verbosity_Diagnostic, "Parser", TOOLS_KO " Operator %s not found\n", operator_token.word_view().c_str());
            lang.ribbon.rollback();
            return nullptr;
        }

        // Precedence check
        if (ope->precedence <= _precedence && _precedence > 0)
        {// always update the first operation if they have the same precedence or less.
            TOOLS_DEBUG_LOG(Verbosity_Diagnostic, "Parser", TOOLS_KO " Has lower precedence\n");
            lang.ribbon.rollback();
            return nullptr;
        }

        // Parse right expression
        if ( Node_Slot* right = lang_parse_expression(lang, parent_scope, ope->precedence) )
        {
            // Create a function signature according to ltype, rtype and operator word
            Type_Descriptor type;
            type_init<any(any, any)>(&type);
            type.name = ope->identifier;
            type.function.args[0].type = _left->property->type;
            type.function.args[1].type = right->property->type;

            Node* binary_op_node = graph_create_operator( lang.graph, &type, _left->node->scope );

            Node::Invokable_Component& binary_op = binary_op_node->component.invokable;

            binary_op.identifier_token = operator_token;
            binary_op.lvalue_in()->property->token.type = _left->property->token.type;
            binary_op.rvalue_in()->property->token.type = right->property->token.type;

            graph_connect_or_merge(_left, binary_op.lvalue_in());
            graph_connect_or_merge(right, binary_op.rvalue_in() );

            lang.ribbon.commit();
            TOOLS_DEBUG_LOG(Verbosity_Diagnostic, "Parser", TOOLS_OK " Binary expression parsed:\n%s\n", lang.ribbon.to_string().c_str());
            return binary_op_node->value_out();
        }

        TOOLS_DEBUG_LOG(Verbosity_Diagnostic, "Parser", TOOLS_KO " Right expression is null\n");
        lang.ribbon.rollback();
        return nullptr;
    }

    Node_Slot* lang_parse_unary_operator_expression(Language& lang, Scope* parent_scope, u8_t _precedence)
    {
        TOOLS_DEBUG_LOG(Verbosity_Diagnostic, "Parser", "parseUnaryOperationExpression...\n");

        if (!lang.ribbon.can_eat(2))
        {
            TOOLS_DEBUG_LOG(Verbosity_Diagnostic, "Parser", TOOLS_KO " Not enough tokens\n");
            return nullptr;
        }

        lang.ribbon.start_transaction();
        Token operator_token = lang.ribbon.eat();

        // Check if we get an operator first
        if (operator_token.type != Token_Type_operator)
        {
            lang.ribbon.rollback();
            TOOLS_DEBUG_LOG(Verbosity_Diagnostic, "Parser", TOOLS_KO " Expecting an operator token first\n");
            return nullptr;
        }

        // Parse expression after the operator
        Node_Slot* out_atomic = lang_parse_atomic_expression( lang, parent_scope );

        if ( !out_atomic )
        {
            out_atomic = lang_parse_parenthesis_expression( lang, parent_scope );
        }

        if ( !out_atomic )
        {
            TOOLS_DEBUG_LOG(Verbosity_Diagnostic, "Parser", TOOLS_KO " Right expression is null\n");
            lang.ribbon.rollback();
            return nullptr;
        }

        // Create a function signature
        Type_Descriptor type;
        type_init<any(any)>(&type);
        type.name = operator_token.word_view();
        type.function.args[0].type = out_atomic->property->type;

        Node* node = graph_create_operator(lang.graph, &type, parent_scope );
        node->component.invokable.identifier_token = operator_token;
        node->component.invokable.lvalue_in()->property->token.type = out_atomic->property->token.type;

        graph_connect_or_merge(out_atomic, node->component.invokable.lvalue_in() );

        TOOLS_DEBUG_LOG(Verbosity_Diagnostic, "Parser", TOOLS_OK " Unary expression parsed:\n%s\n", lang.ribbon.to_string().c_str());
        lang.ribbon.commit();

        return node->value_out();
    }

    Node_Slot* lang_parse_atomic_expression(Language& lang, Scope* parent_scope)
    {
        TOOLS_DEBUG_LOG(Verbosity_Diagnostic, "Parser", "Parsing atomic expression ... \n");

        if (!lang.ribbon.can_eat())
        {
            TOOLS_DEBUG_LOG(Verbosity_Diagnostic, "Parser", TOOLS_KO " Not enough tokens\n");
            return nullptr;
        }

        lang.ribbon.start_transaction();
        Token token = lang.ribbon.eat();

        if (token.type == Token_Type_operator)
        {
            TOOLS_DEBUG_LOG(Verbosity_Diagnostic, "Parser", TOOLS_KO " Cannot start with an operator token\n");
            lang.ribbon.rollback();
            return nullptr;
        }

        if ( Node_Slot* result = lang_token_to_slot( lang, parent_scope, token) )
        {
            lang.ribbon.commit();
            TOOLS_DEBUG_LOG(Verbosity_Diagnostic, "Parser", TOOLS_OK " Atomic expression parsed:\n%s\n", lang.ribbon.to_string().c_str());
            return result;
        }

        lang.ribbon.rollback();
        TOOLS_DEBUG_LOG(Verbosity_Diagnostic,  "Parser", TOOLS_KO " Unable to parse token (%llu)\n", token.index );

        return nullptr;
    }

    Node_Slot* lang_parse_parenthesis_expression(Language& lang, Scope* parent_scope)
    {
        TOOLS_DEBUG_LOG(Verbosity_Diagnostic, "Parser", "parse parenthesis expr...\n");

        if (!lang.ribbon.can_eat())
        {
            TOOLS_DEBUG_LOG(Verbosity_Diagnostic, "Parser", TOOLS_KO " No enough tokens.\n");
            return nullptr;
        }

        lang.ribbon.start_transaction();
        Token currentToken = lang.ribbon.eat();
        if (currentToken.type != Token_Type_parenthesis_open)
        {
            TOOLS_DEBUG_LOG(Verbosity_Diagnostic, "Parser", TOOLS_KO " Open bracket not found.\n");
            lang.ribbon.rollback();
            return nullptr;
        }

        Node_Slot* result = lang_parse_expression(lang, parent_scope);
        if ( result )
        {
            Token token = lang.ribbon.eat();
            if (token.type != Token_Type_parenthesis_close)
            {
                TOOLS_DEBUG_LOG(Verbosity_Diagnostic, "Parser", "%s \n", lang.ribbon.to_string().c_str());
                TOOLS_DEBUG_LOG(Verbosity_Diagnostic, "Parser", TOOLS_KO " Parenthesis close expected\n",
                            token.word_view().c_str());
                lang.ribbon.rollback();
            }
            else
            {
                TOOLS_DEBUG_LOG(Verbosity_Diagnostic, "Parser", TOOLS_OK " Parenthesis expression parsed:\n%s\n", lang.ribbon.to_string().c_str());
                lang.ribbon.commit();
            }
        }
        else
        {
            TOOLS_DEBUG_LOG(Verbosity_Diagnostic, "Parser", TOOLS_KO " No expression after open parenthesis.\n");
            lang.ribbon.rollback();
        }
        return result;
    }

    Node* lang_parse_expression_block(Language& lang, Scope* parent_scope, Node_Slot* flow_out, Node_Slot* value_in)
    {
        lang.ribbon.start_transaction();

        // Parse an expression
        Node_Slot* value_out = lang_parse_expression(lang, parent_scope);

        // When expression value_out is a variable that is already part of the code flow,
        // we must create a variable reference
        if ( value_out && value_out->node->type == Node_Type_VARIABLE )
        {
            Node* variable = value_out->node;

            if ( node_is_connected_to_codeflow(variable) ) // in such case, we have to reference the variable, since a given variable can't be twice (be declared twice) in the codeflow
            {
                // create a new variable reference
                Node* ref_node = graph_create_variable_ref( lang.graph, parent_scope );
                node_variable_ref_set_variable( ref_node, variable );
                // substitute value_out by variable reference's value_out
                value_out = ref_node->value_out();
            }
        }

        if ( !lang.ribbon.can_eat() )
        {
            // we're passing here if there is no more token, which means we reached the end of file.
            // we allow an expression to end like that.
        }
        else
        {
            // However, in case there are still unparsed tokens, we expect certain type of token, otherwise we reset the result
            switch( lang.ribbon.peek().type )
            {
                case Token_Type_end_of_instruction:
                case Token_Type_parenthesis_close:
                    TOOLS_DEBUG_LOG(Verbosity_Diagnostic, "Parser", "End of instruction or parenthesis close: found in next token\n");
                    break;
                default:
                    TOOLS_DEBUG_LOG(Verbosity_Diagnostic, "Parser", TOOLS_KO " End of instruction or parenthesis close expected.\n");
                    value_out = nullptr;
            }
        }

        // When expression value_out is null, but an input was provided,
        // we must create an empty instruction if an end_of_instruction token is found
        if (!value_out && value_in )
        {
            if (lang.ribbon.peek(Token_Type_end_of_instruction))
            {
                TOOLS_DEBUG_LOG(Verbosity_Diagnostic, "Parser", "Empty expression found\n");

                Node* empty_instr = graph_create_empty_instruction( lang.graph, parent_scope );
                value_out = empty_instr->value_out();
            }
        }

        // Ensure value_out is defined or rollback transaction
        if ( !value_out )
        {
            lang.ribbon.rollback();
            TOOLS_DEBUG_LOG(Verbosity_Diagnostic, "Parser", TOOLS_KO " parse instruction\n");
            return nullptr;
        }

        // Connects value_out to the provided input
        if ( value_in )
        {
            graph_connect( value_out, value_in, Graph_Flag_ALLOW_SIDE_EFFECTS);
        }

        // Add an end_of_instruction token as suffix when needed
        if (Token tok = lang.ribbon.eat_if(Token_Type_end_of_instruction))
        {
            value_out->node->suffix = tok;
        }

        // Connects expression flow_in with the provided flow_out
        if ( flow_out != nullptr )
        {
            graph_connect( flow_out, value_out->node->flow_in(), Graph_Flag_ALLOW_SIDE_EFFECTS );
        }

        // Validate transaction
        lang.ribbon.commit();
        TOOLS_DEBUG_LOG(Verbosity_Diagnostic, "Parser", TOOLS_OK " parse instruction:\n%s\n", lang.ribbon.to_string().c_str());

        return value_out->node;
    }

    Scope* lang_parse_program(Language& lang)
    {
        VERIFY(lang.graph != nullptr, "A Graph is expected");

        lang.ribbon.start_transaction();

        Scope* scope = graph_root_scope(lang.graph);

        // Parse main code block
        Node* block_last_node = lang_parse_code_block( lang, scope, scope->node->flow_enter() );

        // To preserve any ignored characters stored in the global token
        // we put the prefix and suffix in resp. token_begin and end.
        Token& tok = lang.ribbon.global_token;

        #warning TODO try to resize prefix/word/suffix instead of pushing stuff (which imply an allocation)
        if(tok.prefix_size) scope->token_begin.prefix_push_front( tok.prefix_view() );
        if(tok.suffix_size) scope->token_end.suffix_push_back( tok.suffix_view() );

        if ( lang.ribbon.can_eat( ) )
        {
            lang.ribbon.rollback();
            graph_reset(lang.graph);
            lang.graph->signal_is_complete.emit();
            TOOLS_LOG(Verbosity_Warning, "Parser", "Some token remains after getting an empty code block\n");
            TOOLS_LOG(Verbosity_Message, "Parser", "Parse program [OK]\n");
            return scope;
        }
        else if ( block_last_node == nullptr )
        {
            TOOLS_LOG(Verbosity_Warning, "Parser", "Program main block is empty\n");
        }

        lang.ribbon.commit();
        lang.graph->signal_is_complete.emit();

        TOOLS_LOG(Verbosity_Message, "Parser", "Parse program [OK]\n");

        return scope;
    }

    Node* lang_parse_scoped_block(Language& lang, Scope* parent_scope, Node_Slot* flow_out)
    {
        ASSERT(parent_scope);
        TOOLS_DEBUG_LOG(Verbosity_Diagnostic, "Parser", "Parsing scoped block ...\n");

        Token token_begin = lang.ribbon.eat_if(Token_Type_scope_begin);
        if ( !token_begin )
        {
            TOOLS_DEBUG_LOG(Verbosity_Diagnostic, "Parser", TOOLS_KO " Expecting root_scope begin token\n");
            return nullptr;
        }

        lang.ribbon.start_transaction();

        Node* node = graph_create_scope(lang.graph, parent_scope);

        if ( flow_out != nullptr )
            graph_connect( flow_out, node->flow_in(), Graph_Flag_ALLOW_SIDE_EFFECTS );


        lang_parse_code_block(lang, node->internal_scope, node->flow_enter()); // no return check, allows empty scope
        Token token_end = lang.ribbon.eat_if(Token_Type_scope_end);

        if ( token_end )
        {
            node->internal_scope->token_begin = token_begin;
            node->internal_scope->token_end = token_end;

            lang.ribbon.commit();
            TOOLS_DEBUG_LOG(Verbosity_Diagnostic, "Parser", TOOLS_OK " Scoped block parsed:\n%s\n", lang.ribbon.to_string().c_str());
            return node;
        }
        else
        {
            TOOLS_DEBUG_LOG(Verbosity_Diagnostic, "Parser", TOOLS_KO " Expecting close root_scope token\n");
        }

        graph_find_and_destroy_node(lang.graph, node);
        lang.ribbon.rollback();
        TOOLS_DEBUG_LOG(Verbosity_Diagnostic, "Parser", TOOLS_KO " Scoped block parsed\n");
        return nullptr;
    }

    Node* lang_parse_code_block(Language& lang, Scope* parent_scope, Node_Slot* flow_out)
    {
        TOOLS_DEBUG_LOG(Verbosity_Diagnostic, "Parser", "Parsing code block...\n" );

        //
        // Parse n atomic code blocks
        //
        lang.ribbon.start_transaction();

        Node_Slot* last_node_flow_out  = flow_out;
        bool     block_end_reached = false;
        size_t   block_size        = 0;

        while (lang.ribbon.can_eat() && !block_end_reached )
        {
            if ( Node* current_block = lang_parse_atomic_code_block( lang, parent_scope, last_node_flow_out) )
            {
                last_node_flow_out = current_block->flow_out();
                ++block_size;
            }
            else
            {
                block_end_reached = true;
            }
        }

        if (last_node_flow_out != nullptr && last_node_flow_out != flow_out )
        {
            lang.ribbon.commit();
            TOOLS_DEBUG_LOG(Verbosity_Diagnostic, "Parser", TOOLS_OK " parse code block:\n%s\n", lang.ribbon.to_string().c_str());
            return last_node_flow_out->node;
        }

        lang.ribbon.rollback();
        TOOLS_DEBUG_LOG(Verbosity_Diagnostic, "Parser", TOOLS_KO " parse code block. Block size is %llu\n", block_size );
        return nullptr;
    }

    Node_Slot* lang_parse_expression(Language& lang, Scope* parent_scope, u8_t _precedence, Node_Slot* _left_override)
    {
        TOOLS_DEBUG_LOG(Verbosity_Diagnostic, "Parser", "Parsing expression ...\n");

        /*
            Get the left-handed operand
        */
        Node_Slot* left = _left_override;

        if (!lang.ribbon.can_eat())
        {
            TOOLS_DEBUG_LOG(Verbosity_Diagnostic, "Parser", TOOLS_OK " Last token reached\n");
            return left;
        }

        if ( !left ) left = lang_parse_parenthesis_expression(lang, parent_scope);
        if ( !left ) left = lang_parse_unary_operator_expression(lang, parent_scope, _precedence);
        if ( !left ) left = lang_parse_function_call(lang, parent_scope);
        if ( !left ) left = lang_parse_variable_declaration(lang, parent_scope); // nullptr => variable won't be attached on the codeflow, it's a part of an expression..
        if ( !left ) left = lang_parse_atomic_expression(lang, parent_scope);

        if (!lang.ribbon.can_eat())
        {
            TOOLS_DEBUG_LOG(Verbosity_Diagnostic, "Parser", TOOLS_OK " Last token reached\n");
            return left;
        }

        if ( !left )
        {
            TOOLS_DEBUG_LOG(Verbosity_Diagnostic, "Parser", TOOLS_OK " Left side is null, we return it\n");
            return left;
        }

        /*
            Get the right-handed operand
        */
        Node_Slot* expression_out = lang_parse_binary_operator_expression(lang, parent_scope, _precedence, left );
        if ( expression_out )
        {
            if (!lang.ribbon.can_eat())
            {
                TOOLS_DEBUG_LOG(Verbosity_Diagnostic, "Parser", TOOLS_OK " Right side parsed, and last token reached\n");
                return expression_out;
            }
            TOOLS_DEBUG_LOG(Verbosity_Diagnostic, "Parser", TOOLS_OK " Right side parsed, continue with a recursive call...\n");
            return lang_parse_expression(lang, parent_scope, _precedence, expression_out);
        }

        TOOLS_DEBUG_LOG(Verbosity_Diagnostic, "Parser", TOOLS_OK " Returning left side only\n");

        return left;
    }

    bool _lang_is_syntax_valid(const Language& lang)
    {
        // TODO: optimization: is this function really useful ? It check only few things.
        //                     The parsing steps that follow (parseProgram) is doing a better check, by looking to what exist in the Language.
        bool success = true;
        auto token = lang.ribbon.cbegin();
        short int opened = 0;

        while (token != lang.ribbon.cend() && success)
        {
            switch (token->type)
            {
                case Token_Type_parenthesis_open:
                {
                    opened++;
                    break;
                }
                case Token_Type_parenthesis_close:
                {
                    if (opened <= 0)
                    {
                        const size_t token_count = 10;
                        const size_t begin       = token->index < token_count ? 0 : token->index - token_count;
                        const size_t end         = token->index + 1;
                        TOOLS_LOG(
                            Verbosity_Error,
                            "Parser",
                            "Syntax Error: Unexpected close bracket after \"... %s\" (position %llu)\n",
                            lang.ribbon.range_to_string(begin, end).c_str(),
                            token->char_position()
                        );
                        success = false;
                    }
                    opened--;
                    break;
                }
                default:
                    break;
            }

            std::advance(token, 1);
        }

        if (opened > 0)// same opened/closed parenthesis count required.
        {
            TOOLS_LOG(Verbosity_Error, "Parser", "Syntax Error: Bracket count mismatch, %i still opened.\n", opened);
            success = false;
        }

        return success;
    }

    bool lang_tokenize(Language& lang, const bdc::String& str)
    {
        lang.buffer = str;
        lang.ribbon.reset( str );
        return lang_tokenize(lang);
    }

    bool lang_tokenize(Language& lang)
    {
        TOOLS_LOG(Verbosity_Diagnostic, "Parser", "Tokenization ...\n");

        bdc::String remainder = lang.buffer;
        size_t ignored_chars_count = 0;

        while ( !remainder.empty() )
        {
            Token  new_token = lang_parse_token( lang,  remainder );

            if ( !new_token )
            {
                TOOLS_LOG(
                    Verbosity_Warning, "Parser", 
                    TOOLS_KO " Unable to tokenize from \"%20s...\" (at char %llu)\n", 
                    remainder.c_str(), (u64_t)remainder.data - (u64_t)lang.buffer.data );
                return false;
            }

            // accumulate ignored chars (see else case to know why)
            if(new_token.type == Token_Type_ignore)
            {
                if ( lang.ribbon.empty() )
                {
                    lang.ribbon.global_token.prefix_end_grow(new_token.size() );
                    continue;
                }

                ignored_chars_count += new_token.size();
                continue;
            }

            if ( ignored_chars_count )
            {
                // case 1: if token type allows it => increase last token's prefix to wrap the ignored chars
                Token& back = lang.ribbon.back();
                if ( _lang_accepts_suffix(lang, back.type) )
                {
                    back.suffix_end_grow(ignored_chars_count);
                    TOOLS_DEBUG_LOG(Verbosity_Diagnostic, "Parser", "      \"%s\" (update) \n", back.view().c_str() );
                }
                // case 2: increase prefix of the new_token up to wrap the ignored chars
                else if ( new_token )
                {
                    new_token.prefix_begin_grow(ignored_chars_count);
                }
                ignored_chars_count = 0;
            }

            lang.ribbon.push(new_token);
            TOOLS_DEBUG_LOG(Verbosity_Diagnostic, "Parser", "%4llu) \"%s\" \n", new_token.index, new_token.view().c_str() );
        }

        // Append remaining ignored chars to the ribbon's suffix
        if ( ignored_chars_count )
        {
            TOOLS_DEBUG_LOG(Verbosity_Diagnostic, "Parser", "Found ignored chars after tokenize, adding to the tokens suffix...\n");
            Token& tok = lang.ribbon.global_token;
            tok.suffix_begin_grow( ignored_chars_count );
        }

        TOOLS_LOG(Verbosity_Diagnostic, "Parser", TOOLS_OK " Tokenization.\n%s\n", lang.ribbon.to_string().c_str() );

        return true;
    }

    Token lang_parse_token(const Language& lang, bdc::String& buffer)
    {
        ASSERT(buffer.size > 0);

        // single-line comment
        if ( buffer.size > 2 && buffer[0] == '/' && buffer[1] == '/')
        {
            u32_t cursor = 2;            
            while ( cursor < buffer.size && buffer[cursor] != '\n' )
            {
                cursor += 1;
            }
            cursor += 1;
            
            String word = {
                buffer.data,
                cursor
            };

            buffer = {
                buffer.data + cursor,
                cursor > buffer.size ? 0 : buffer.size - cursor 
            };
            return Token{ Token_Type_ignore, word };
        }
        
        //  multi-line comment
        if ( buffer.size > 4 && buffer[0] == '/' && buffer[1] == '*' && buffer[2] != '/') // requires "/*" + 1 char that is not /, minimal comment is "/**/", "/*/" is invalid
        {
            u32_t cursor = 2;            
            while ( true )
            {
                if( buffer.size <= cursor )
                    return Token{ Token_Type_NULL, { buffer.data + cursor, buffer.size - cursor } };
                if (buffer[cursor-1] == '*' && buffer[cursor] != '/')
                    break;
                cursor += 1;
            }
            cursor += 1;
            
            String word = {
                buffer.data,
                cursor
            };

            buffer = {
                buffer.data + cursor,
                cursor > buffer.size ? 0 : buffer.size - cursor 
            };
            return Token{ Token_Type_ignore, word };
        }

        // single-char
        auto single_char_found = lang.token_type_by_single_char.find(buffer[0]); // index lookup
        if( single_char_found != lang.token_type_by_single_char.end() )
        {
            String word = bdc::string_lsplit( buffer, 1);

            bdc::string_advance(buffer, word.size );

            return Token{ single_char_found->second, word };
        }

        // operators
        switch ( buffer[0] )
        {
            case '=':
            {
                bdc::String word;

                // Double char operators starting with "=" ("=>" or "==")
                if (buffer.size >= 1 && (buffer[1] == '>' || buffer[1] == '='))
                {
                    word = bdc::string_lsplit(buffer, 2);
                }
                // "="
                else
                {
                    word = bdc::string_lsplit(buffer, 1);
                }

                string_advance(buffer, word.size);
                return Token{ Token_Type_operator, word };
            }

            case '!':
            case '/':
            case '*':
            case '+':
            case '-':
            case '>':
            case '<':
            {
                // "<operator>=" (do not handle: "++", "--")
                if (buffer.size > 1)
                {
                    // 3-chars operators:
                    // This is just a single special case for equivalence operator ("<=>")
                    // we MUST parse this before "<=" of course, since "<=>" includes "<="
                    if (buffer.size > 2 && buffer[0] == '<'  && buffer[1] == '=' && buffer[2] == '>'  )
                    {
                        bdc::String word = bdc::string_lsplit(buffer, 3);
                        bdc::string_advance(buffer, word.size);
                        return Token{ Token_Type_operator, word };
                    }

                    // 2-chars operators: >=, <= += -=, etc.
                    if (buffer[1] == '=')
                    {                    
                        bdc::String word = bdc::string_lsplit(buffer, 2);
                        bdc::string_advance(buffer, word.size);
                        return Token{ Token_Type_operator, word };
                    }
                }

                // single char operator
                bdc::String word = bdc::string_lsplit(buffer, 1);
                bdc::string_advance(buffer, 1);
                return Token{ Token_Type_operator, word };
            }
        }

        // number (double)
        //     note: we accept zeros as prefix (ex: "0002.15454", or "01012")
        if ( std::isdigit( buffer[0] ) )
        {
            u32_t cursor = 1;
            Token_Type type = Token_Type_literal_int;

            // integer
            while (cursor != buffer.size && std::isdigit( buffer[cursor] ))
            {
                ++cursor;
            }

            // double
            if(cursor + 1 < buffer.size
            && buffer[cursor] == '.'      // has a decimal separator
                && std::isdigit(buffer[cursor + 1]) // followed by a digit
            )
            {
                u32_t local_cursor_decimal_separator = cursor;
                ++cursor;

                // decimal portion
                while (cursor != buffer.size && std::isdigit(buffer[cursor]))
                {
                    ++cursor;
                }
                type = Token_Type_literal_double;
            }
            bdc::String word = bdc::string_lsplit(buffer, cursor);
            bdc::string_advance(buffer, cursor);
            return Token{type, word };
        }

        // double-quoted string
        if ( buffer[0] == '"')
        {
            u32_t cursor = 1;

            while (cursor != buffer.size && (buffer[cursor] != '"' || buffer[cursor - 1] == '\\'))
            {
                ++cursor;
            }

            if( buffer[cursor] != '"' )
            {
                return Token{ Token_Type_NULL };
            }
            
            ++cursor;
            bdc::String word = bdc::string_lsplit(buffer, cursor);
            bdc::string_advance(buffer, cursor);
            return Token{Token_Type_literal_string, word};
        }

        // symbol (identifier or keyword)
        if ( std::isalpha( buffer[0] ) || buffer[0] == '_' )
        {
            // parse symbol
            u32_t cursor = 1;
            while (cursor < buffer.size && (std::isalnum( buffer[cursor]) || buffer[cursor] == '_') )
            {
                ++cursor;
            }
            
            bdc::String word = bdc::string_lsplit(buffer, cursor );
            bdc::string_advance(buffer, cursor);

            // symbol might be a reserved keyword, let's seach in the keyword index...
            String_Hash word_hash = string_hash(word);
            auto keyword_found = lang.token_type_by_keyword.find( word_hash.hash );
            if (keyword_found != lang.token_type_by_keyword.end())
            {            
                return Token{ keyword_found->second, word };
            }

            // ...otherwise, symbol is an identifier
            return Token{ Token_Type_identifier, word};
            
        }
        return Token{ Token_Type_NULL };
    }

    Node_Slot* lang_parse_function_call(Language& lang, Scope* parent_scope)
    {
        TOOLS_DEBUG_LOG(Verbosity_Diagnostic, "Parser", "parse function call...\n");

        // Check if the minimum token count required is available ( 0: identifier, 1: open parenthesis, 2: close parenthesis)
        if (!lang.ribbon.can_eat(3))
        {
            TOOLS_DEBUG_LOG(Verbosity_Diagnostic, "Parser", TOOLS_KO " 3 tokens min. are required\n");
            return nullptr;
        }

        lang.ribbon.start_transaction();

        // Try to parse regular function: function(...)
        bdc::String function_identifier;
        Token token_0 = lang.ribbon.eat();
        Token token_1 = lang.ribbon.eat();
        if (token_0.type == Token_Type_identifier &&
            token_1.type == Token_Type_parenthesis_open)
        {
            function_identifier = token_0.word_view();
            TOOLS_DEBUG_LOG(Verbosity_Diagnostic, "Parser", TOOLS_OK " Regular function pattern detected.\n");
        }
        else // Try to parse operator like (ex: operator==(..,..))
        {
            Token token_2 = lang.ribbon.eat();// eat a "supposed open bracket>

            if (token_0.type == Token_Type_keyword_operator && token_1.type == Token_Type_operator && token_2.type == Token_Type_parenthesis_open)
            {
                function_identifier = token_1.word_view();// operator
                TOOLS_DEBUG_LOG(Verbosity_Diagnostic, "Parser", TOOLS_OK " Operator function-like pattern detected.\n");
            }
            else
            {
                TOOLS_DEBUG_LOG(Verbosity_Diagnostic, "Parser", TOOLS_KO " Not a function.\n");
                lang.ribbon.rollback();
                return nullptr;
            }
        }
        std::vector<Node_Slot*> result_slots;

        // Declare a new function prototype
        Type_Descriptor function_type;
        type_init<any()>(&function_type);
        function_type.name = function_identifier;

        bool parsingError = false;
        while (!parsingError && lang.ribbon.can_eat() &&
            lang.ribbon.peek().type != Token_Type_parenthesis_close)
        {
            Node_Slot* expression_out = lang_parse_expression(lang, parent_scope);
            if ( expression_out )
            {
                result_slots.push_back( expression_out );
                function_type.function_push_arg( expression_out->property->type );
                lang.ribbon.eat_if(Token_Type_list_separator);
            }
            else
            {
                parsingError = true;
            }
        }

        // eat "close bracket supposed" token
        if ( !lang.ribbon.eat_if(Token_Type_parenthesis_close) )
        {
            TOOLS_LOG(Verbosity_Warning, "Parser", TOOLS_KO " Expecting parenthesis close\n");
            lang.ribbon.rollback();
            return nullptr;
        }


        // Find the prototype in the language library
        Node* fct_node = graph_create_function( lang.graph, &function_type, parent_scope );

        for ( int i = 0; i < fct_node->component.invokable.argument_slots.size; i++ )
        {
            // Connects each results to the corresponding input
            graph_connect_or_merge(result_slots.at(i), fct_node->component.invokable.argument_slots[i] );
        }

        lang.ribbon.commit();
        TOOLS_DEBUG_LOG(Verbosity_Diagnostic, "Parser", TOOLS_KO " Function call parsed:\n%s\n", lang.ribbon.to_string().c_str() );

        return fct_node->value_out();
    }

    Node* lang_parse_if_block(Language& lang, Scope* parent_scope, Node_Slot* flow_out)
    {
        lang.ribbon.start_transaction();

        Token if_token = lang.ribbon.eat_if(Token_Type_keyword_if);
        if ( !if_token )
        {
            return nullptr;
        }

        TOOLS_DEBUG_LOG(Verbosity_Diagnostic, "Parser", "Parsing conditional structure...\n");

        bool    success  = false;
        Node*   if_node  = graph_create_cond_struct( lang.graph, parent_scope );
        if_node->component.branching.branch_prefix = lang.ribbon.get_eaten();

        graph_connect(flow_out, if_node->flow_in(), Graph_Flag_ALLOW_SIDE_EFFECTS );

        if (lang.ribbon.eat_if(Token_Type_parenthesis_open) )
        {
            TOOLS_DEBUG_LOG(Verbosity_Diagnostic, "Parser", "Parsing conditional structure's condition...\n");

            // condition
            lang_parse_expression_block( lang, if_node->internal_scope, nullptr, if_node->component.branching.condition_in());

            if (lang.ribbon.eat_if(Token_Type_parenthesis_close) )
            {
                // scope
                Node* block = lang_parse_atomic_code_block( lang,  if_node->internal_scope, if_node->component.branching.branch_out(Branch_TRUE) );

                if ( block )
                {
                    // else
                    if ( lang.ribbon.eat_if(Token_Type_keyword_else) )
                    {
                        if_node->component.branching.branch_suffix = lang.ribbon.get_eaten();

                        if ( Node* else_block = lang_parse_atomic_code_block( lang,  if_node->internal_scope, if_node->component.branching.branch_out(Branch_FALSE) ) )
                        {
                            success = true;
                            TOOLS_DEBUG_LOG(Verbosity_Diagnostic, "Parser", TOOLS_OK " else block parsed.\n");
                        }
                        else
                        {
                            TOOLS_DEBUG_LOG(Verbosity_Diagnostic, "Parser", TOOLS_KO " Single instruction or root_scope expected\n");
                        }
                    }
                    else
                    {
                        success = true;
                    }
                }
                else
                {
                    TOOLS_DEBUG_LOG(Verbosity_Diagnostic, "Parser", TOOLS_KO " Single instruction or root_scope expected\n");
                }
            }
            else
            {
                TOOLS_DEBUG_LOG(Verbosity_Diagnostic, "Parser", TOOLS_KO " Close bracket expected\n");
            }
        }

        if ( success )
        {
            lang.ribbon.commit();
            TOOLS_DEBUG_LOG(Verbosity_Diagnostic, "Parser", TOOLS_OK " Parse conditional structure:\n%s\n", lang.ribbon.to_string().c_str() );
            // TODO: connect true/false branches flow_out to scope flow_leave?"
            return if_node;
        }

        graph_find_and_destroy_node(lang.graph, if_node);
        lang.ribbon.rollback();
        TOOLS_DEBUG_LOG(Verbosity_Diagnostic, "Parser", TOOLS_KO " Parse conditional structure \n");

        return {};
    }

    Node* lang_parse_for_block(Language& lang, Scope* parent_scope, Node_Slot* flow_out)
    {
        bool        success     = false;
        Node*    for_node    = nullptr;

        lang.ribbon.start_transaction();

        if ( Token token_for = lang.ribbon.eat_if(Token_Type_keyword_for) )
        {
            TOOLS_DEBUG_LOG(Verbosity_Diagnostic, "Parser", "Parsing for loop ...\n");

            for_node = graph_create_for_loop( lang.graph, parent_scope );
            for_node->component.branching.branch_prefix = token_for;

            graph_connect( flow_out, for_node->flow_in(), Graph_Flag_ALLOW_SIDE_EFFECTS );

            Token open_bracket = lang.ribbon.eat_if(Token_Type_parenthesis_open);
            if ( open_bracket)
            {
                TOOLS_DEBUG_LOG(Verbosity_Diagnostic, "Parser", "Parsing for set_name/condition/iter instructions ...\n");

                // first we parse three instructions, no matter if we find them, we'll continue (we are parsing something abstract)

                // parse init; condition; iteration or nothing
                lang_parse_expression_block(lang, for_node->internal_scope, nullptr, for_node->component.branching.initialization_slot)
                && lang_parse_expression_block(lang, for_node->internal_scope, nullptr, for_node->component.branching.condition_in())
                && lang_parse_expression_block(lang, for_node->internal_scope, nullptr, for_node->component.branching.iteration_slot);

                // parse parenthesis close
                if ( Token parenthesis_close = lang.ribbon.eat_if(Token_Type_parenthesis_close) )
                {
                    Node* block = lang_parse_atomic_code_block( lang,  for_node->internal_scope, for_node->component.branching.branch_out(Branch_TRUE) ) ;

                    if ( block )
                    {
                        success = true;
                        TOOLS_DEBUG_LOG(Verbosity_Diagnostic, "Parser", "Scope or single instruction found\n");
                    }
                    else
                    {
                        TOOLS_DEBUG_LOG(Verbosity_Diagnostic, "Parser", TOOLS_KO " Scope or single instruction expected\n");
                    }
                }
                else
                {
                    TOOLS_DEBUG_LOG(Verbosity_Diagnostic, "Parser", TOOLS_KO " Close parenthesis was expected.\n");
                }
            }
            else
            {
                TOOLS_DEBUG_LOG(Verbosity_Diagnostic, "Parser", TOOLS_KO " Open parenthesis was expected.\n");
            }
        }

        if ( success )
        {
            TOOLS_DEBUG_LOG(Verbosity_Diagnostic, "Parser", TOOLS_OK " For block parsed\n");
            lang.ribbon.commit();
            // TODO: Should we connect true/false branches to scope's flow_leave Node_Slot?
            return for_node;
        }

        if ( for_node )
        {
            graph_find_and_destroy_node(lang.graph, for_node);
        }
        lang.ribbon.rollback();
        TOOLS_DEBUG_LOG(Verbosity_Diagnostic, "Parser", TOOLS_KO " Could not parse for block\n");
        return {};
    }

    Node* lang_parse_while_block(Language& lang, Scope* parent_scope, Node_Slot* flow_out)
    {
        bool        success     = false;
        Node*    while_node  = nullptr;
        Node*    block       = nullptr;

        lang.ribbon.start_transaction();

        if ( Token token_while = lang.ribbon.eat_if(Token_Type_keyword_while) )
        {
            TOOLS_DEBUG_LOG(Verbosity_Diagnostic, "Parser", "Parsing while ...\n");

            while_node = graph_create_while_loop( lang.graph, parent_scope );
            while_node->component.branching.branch_prefix = token_while;

            graph_connect( flow_out, while_node->flow_in(), Graph_Flag_ALLOW_SIDE_EFFECTS );

            if ( Token open_bracket = lang.ribbon.eat_if(Token_Type_parenthesis_open) )
            {
                TOOLS_DEBUG_LOG(Verbosity_Diagnostic, "Parser", "Parsing while condition ... \n");

                // Parse an optional condition
                lang_parse_expression_block( lang, while_node->internal_scope, nullptr, while_node->component.branching.condition_in());

                if (lang.ribbon.eat_if(Token_Type_parenthesis_close) )
                {
                    block = lang_parse_atomic_code_block( lang,  while_node->internal_scope, while_node->component.branching.branch_out(Branch_TRUE) );
                    if ( block )
                    {
                        success = true;
                    }
                    else
                    {
                        TOOLS_DEBUG_LOG(Verbosity_Diagnostic, "Parser", TOOLS_KO "  Scope or single instruction expected\n");
                    }
                }
                else
                {
                    TOOLS_DEBUG_LOG(Verbosity_Diagnostic, "Parser", TOOLS_KO "  Parenthesis close expected\n");
                }
            }
            else
            {
                TOOLS_DEBUG_LOG(Verbosity_Diagnostic, "Parser", TOOLS_KO "  Parenthesis close expected\n");
            }
        }

        if ( success )
        {
            TOOLS_DEBUG_LOG(Verbosity_Diagnostic, "Parser", "Parsing while:\n%s\n", lang.ribbon.to_string().c_str() );
            lang.ribbon.commit();
            // TODO: Should we connect true/false branches to scope's flow_leave SLot?
            return while_node;
        }

        lang.ribbon.rollback();
        graph_find_and_destroy_node(lang.graph, while_node);
        graph_find_and_destroy_node(lang.graph, block);

        return {};
    }

    Node* lang_parse_return(Language& lang, Scope* parent_scope, Node_Slot* flow_out)
    {
        if (!lang.ribbon.can_eat(2))
        {
            return nullptr;
        }

        lang.ribbon.start_transaction();

        if ( Token return_token = lang.ribbon.eat_if(Token_Type_keyword_return) )
        {
            // Parse the expression at the right side of the return
            if ( Node_Slot* expression_out = lang_parse_expression(lang, parent_scope) )
            {
                const Type_Descriptor* type = expression_out->property->type;
                Node* return_node = graph_create_return( lang.graph, type, parent_scope );
                return_node->value->token = return_token;

                // TODO: assign prefix and suffix to return Node

                // Connect the expression to the return Node
                graph_connect(expression_out, return_node->value_in());
                // and to the flow
                graph_connect(flow_out, return_node->flow_in());

                lang.ribbon.commit();
                return return_node;
            }
        }

        lang.ribbon.rollback();
        return nullptr;
    }

    Node_Slot* lang_parse_variable_declaration(Language& lang, Scope* parent_scope)
    {
        if (!lang.ribbon.can_eat(2))
        {
            return nullptr;
        }

        lang.ribbon.start_transaction();

        bool  success          = false;
        Token type_token       = lang.ribbon.eat();
        Token identifier_token = lang.ribbon.eat();

        if (type_token.is_keyword_type() && identifier_token.type == Token_Type_identifier)
        {
            const Type_Descriptor* type = lang_get_type(lang, type_token.type);
            Node* variable_node = graph_create_variable( lang.graph, type, identifier_token.word_view(), parent_scope );

            SET_FLAGS(variable_node->component.variable.flags, VariableFlag_DECLARED);
            variable_node->component.variable.type_token = type_token;
            node_set_identifier_token(variable_node, identifier_token );

            // declaration with assignment ?
            Token operator_token = lang.ribbon.eat_if(Token_Type_operator);
            if (operator_token && operator_token.word_view() == "=")
            {
                // an expression is expected
                if ( Node_Slot* expression_out = lang_parse_expression(lang, parent_scope) )
                {
                    // expression's out ----> variable's in
                    graph_connect_to_variable(expression_out, variable_node );

                    variable_node->component.variable.operator_token = operator_token;
                    success = true;
                }
                else
                {
                    TOOLS_DEBUG_LOG(
                        Verbosity_Diagnostic, "Parser", 
                        TOOLS_KO "  Initialization expression expected for %s\n", identifier_token.word_view().c_str());
                }
            }
                // Declaration without assignment
            else
            {
                success = true;
            }

            if ( success )
            {
                TOOLS_DEBUG_LOG(Verbosity_Diagnostic, "Parser", TOOLS_OK " Variable declaration: %s %s\n",
                            variable_node->value->type->name.c_str(),
                            identifier_token.word_view().c_str());
                lang.ribbon.commit();
                return variable_node->value_out();
            }

            TOOLS_DEBUG_LOG(
                Verbosity_Diagnostic, "Parser", 
                TOOLS_KO "  Initialization expression expected for %s\n", identifier_token.word_view().c_str());
            graph_find_and_destroy_node(lang.graph, variable_node);
        }

        lang.ribbon.rollback();
        return nullptr;
    }

    //---------------------------------------------------------------------------------------------------------------------------
    // [SECTION] C. Serializer --------------------------------------------------------------------------------------------------
    //---------------------------------------------------------------------------------------------------------------------------

    const Node_Slot* lang_serialize_invokable(const Language& lang, bdc::String_Builder& out, const Node* _node)
    {
        if (_node->type == Node_Type_OPERATOR )
        {
            Array<Node_Slot*> args = array_view( _node->component.invokable.argument_slots );
            int precedence = lang_get_precedence(lang, &_node->component.invokable.type);

            switch ( _node->component.invokable.type.function.args.size )
            {
                case 2:
                {
                    // Left part of the expression
                    {
                        const Type_Descriptor* l_func_type = node_get_connected_function_type(_node, LEFT_VALUE_PROPERTY);
                        bool needs_braces = l_func_type && lang_get_precedence(lang, l_func_type) < precedence;
                        Serialization_Flags flags = Serialization_Flag_RECURSE
                                            | needs_braces * Serialization_Flag_WRAP_WITH_BRACES ;
                        lang_serialize_input( lang, out, args[0], flags );
                    }

                    // Operator
                    VERIFY( _node->component.invokable.identifier_token, "identifier token should have been assigned in parse_function_call");
                    string_builder_append( out, lang_serialize_token( lang, _node->component.invokable.identifier_token ));

                    // Right part of the expression
                    {
                        const Type_Descriptor* r_func_type = node_get_connected_function_type(_node, RIGHT_VALUE_PROPERTY);
                        bool needs_braces = r_func_type && lang_get_precedence(lang, r_func_type) < precedence;
                        Serialization_Flags flags = Serialization_Flag_RECURSE
                                            | needs_braces * Serialization_Flag_WRAP_WITH_BRACES ;
                        lang_serialize_input( lang, out, args[1], flags );
                    }
                    break;
                }

                case 1:
                {
                    // operator ( ... innerOperator ... )   ex:   -(a+b)

                    ASSERT( _node->component.invokable.identifier_token );
                    string_builder_append( out, lang_serialize_token( lang, _node->component.invokable.identifier_token) );

                    bool needs_braces    = node_get_connected_function_type(_node, LEFT_VALUE_PROPERTY) != nullptr;
                    Serialization_Flags flags = Serialization_Flag_RECURSE
                                        | needs_braces * Serialization_Flag_WRAP_WITH_BRACES;
                    lang_serialize_input( lang, out, args[0], flags );
                    break;
                }
            }
        }
        else
        {
            lang_serialize_func_call(lang, out, &_node->component.invokable.type, array_view(_node->component.invokable.argument_slots) );
        }

        return _node->value_out();
    }

    bdc::String_Builder& lang_serialize_func_call(const Language& lang, bdc::String_Builder& out, const Type_Descriptor *function_type, const bdc::Array<Node_Slot*>& inputs)
    {
        string_builder_append( out, function_type->name );
        
        string_builder_append( out, lang_serialize_token_type_default(lang, Token_Type_parenthesis_open));

        for (const Node_Slot* input_slot : inputs)
        {
            ASSERT( HAS_FLAGS(input_slot->flags, Node_Slot::Flag_INPUT) );
            if ( input_slot != inputs[0])
            {
                string_builder_append( out, lang_serialize_token_type_default(lang, Token_Type_list_separator));
            }
            lang_serialize_input( lang, out, input_slot, Serialization_Flag_RECURSE );
        }

        string_builder_append( out, lang_serialize_token_type_default(lang, Token_Type_parenthesis_close) );
        return out;
    }

    bdc::String_Builder &lang_serialize_func_sig(const Language& lang, bdc::String_Builder& out, const Type_Descriptor *function_type)
    {
        string_builder_append( out, lang_serialize_type(lang, function_type->function.return_type));
        string_builder_append( out, " ");
        string_builder_append( out, function_type->name );
        string_builder_append( out, lang_serialize_token_type_default(lang, Token_Type_parenthesis_open) );

        for (auto it = function_type->function.args.begin(); it != function_type->function.args.end(); it++)
        {
            if (it != function_type->function.args.begin())
            {
                string_builder_append( out, lang_serialize_token_type_default(lang, Token_Type_list_separator));
                string_builder_append( out, " ");
            }
            string_builder_append( out, lang_serialize_type(lang, (*it).type) );
        }

        string_builder_append( out, lang_serialize_token_type_default(lang, Token_Type_parenthesis_close));
        return out;
    }

    bdc::String_Builder& lang_serialize_variable_ref(const Language& lang, bdc::String_Builder& out, const Node* _node)
    {
        ASSERT(_node->type == Node_Type_VARIABLE_REF);
        String token_str = lang_serialize_token( lang, node_get_identifier_token(_node) );
        string_builder_append( out, token_str);
        return out;
    }

    bdc::String_Builder& lang_serialize_variable(const Language& lang, bdc::String_Builder& out, const Node *_node)
    {
        ASSERT(_node->type == Node_Type_VARIABLE);

        // 1. Serialize variable's type

        // If parsed
        if ( _node->component.variable.type_token )
        {
            string_builder_append(out, lang_serialize_token( lang, _node->component.variable.type_token) );
        }
        else // If created in the graph by the user
        {
            string_builder_append( out, lang_serialize_type(lang, _node->value->type) );
            string_builder_append(out, " ");
        }

        // 2. Serialize variable identifier
        string_builder_append(out, lang_serialize_token( lang, node_get_identifier_token(_node) ));

        // 3. Initialisation
        //    When a VariableNode has its input connected, we serialize it as its initialisation expression

        const Node_Slot* slot = _node->value_in();
        if ( slot->adjacent.size != 0 )
        {
            if ( _node->component.variable.operator_token )
                string_builder_append(out, _node->component.variable.operator_token.view());
            else
                string_builder_append(out, " = ");

            lang_serialize_input( lang, out, slot, Serialization_Flag_RECURSE );
        }
        return out;
    }

    bdc::String_Builder& lang_serialize_return(const Language& lang, bdc::String_Builder& out, const Node* node)
    {
        ASSERT(node->type == Node_Type_RETURN);

        if( node->value->token )
        {
            string_builder_append(out, lang_serialize_token( lang, node->value->token ));
        }
        else
        {
            string_builder_append(out, lang.keyword_by_token_type.at(Token_Type_keyword_return) );
            string_builder_append(out, " ");
        }

        if ( const Node_Slot* input_slot = node->value_in() )
        {
            lang_serialize_input( lang, out, input_slot, Serialization_Flag_RECURSE );
        }

        return out;
    }

    bdc::String_Builder &lang_serialize_input(const Language& lang, bdc::String_Builder& out, const Node_Slot* slot, Serialization_Flags _flags )
    {
        ASSERT( HAS_FLAGS(slot->flags, Node_Slot::Flag_INPUT ) );

        const Node_Slot*     adjacent_slot     = slot->first_adjacent();
        const Node_Property* adjacent_property = adjacent_slot != nullptr ? adjacent_slot->property
                                                                            : nullptr;
        // Append open brace?
        if ( _flags & Serialization_Flag_WRAP_WITH_BRACES )
            string_builder_append(out, lang_serialize_token_type_default(lang,  Token_Type_parenthesis_open));

        if ( adjacent_property == nullptr )
        {
            // Simply serialize this property
            lang_serialize_property(lang, out, slot->property);
        }
        else
        {
            VERIFY( _flags & Serialization_Flag_RECURSE, "Why would you call serialize_input without RECURSE flag?");
            // Append token prefix?
            if (adjacent_property->token)
                string_builder_append(out, adjacent_property->token.prefix_view());

            // Serialize adjacent slot
            lang_serialize_value_out(lang, out, adjacent_slot, Serialization_Flag_RECURSE);

            // Append token suffix?
            if (adjacent_property->token )
                    string_builder_append(out, adjacent_property->token.suffix_view());
        }

        // Append close brace?
        if ( _flags & Serialization_Flag_WRAP_WITH_BRACES )
            string_builder_append( out, lang_serialize_token_type_default(lang, Token_Type_parenthesis_close));

        return out;
    }

    bdc::String_Builder& lang_serialize_value_out(const Language& lang, bdc::String_Builder& out, const Node_Slot* slot, Serialization_Flags _flags)
    {
        // If output is node's output value, we serialize the node
        if( slot == slot->node->value_out() )
        {
            return lang_serialize_node(lang, out, slot->node, _flags);
        }

        // Otherwise, it might be a variable reference, so we serialize the identifier only
        ASSERT(slot->node->type == Node_Type_VARIABLE ); // Can't be another type
        VERIFY( slot == slot->node->component.variable.ref_out, "Cannot serialize an other slot from a VariableNode");
        return string_builder_append( out, node_get_identifier(slot->node) );
    }

    bdc::String_Builder& lang_serialize_node(const Language& lang, bdc::String_Builder& out, const Node* node, Serialization_Flags _flags )
    {
        if ( node == nullptr )
            return out;

        ASSERT( _flags == Serialization_Flag_RECURSE ); // The only flag configuration handled for now

        switch ( node->type )
        {
            case Node_Type_RETURN:            lang_serialize_return(lang, out, node ); break;
            case Node_Type_IF_ELSE:           lang_serialize_cond_struct(lang, out, node );             break;
            case Node_Type_FOR_LOOP:          lang_serialize_for_loop(lang, out, node );                break;
            case Node_Type_WHILE_LOOP:        lang_serialize_while_loop(lang, out, node );              break;
            case Node_Type_LITERAL:           lang_serialize_literal(lang, out, node );                 break;
            case Node_Type_VARIABLE:          lang_serialize_variable(lang, out, node );                break;
            case Node_Type_VARIABLE_REF:      lang_serialize_variable_ref(lang, out, node );            break;
            case Node_Type_FUNCTION:          [[fallthrough]];        
            case Node_Type_OPERATOR:          lang_serialize_invokable(lang, out, node );               break;
            case Node_Type_EMPTY_INSTRUCTION: lang_serialize_empty_instruction(lang, out, node);        break;
            case Node_Type_ROOT:              [[fallthrough]];
            case Node_Type_SCOPE:             lang_serialize_scope(lang, out, node->internal_scope ); break;
            default:                          VERIFY(false, "Unhandled NodeType, can't serialize");
        }

        return string_builder_append( out, lang_serialize_token( lang, node->suffix ));
    }

    bdc::String_Builder& lang_serialize_scope(const Language& lang, bdc::String_Builder& out, const Scope* scope)
    {
        string_builder_append( out, lang_serialize_token( lang, scope->token_begin) );
        
        for(Node* node : scope_get_backbone(scope) )
        {
            lang_serialize_node( lang, out, node, Serialization_Flag_RECURSE);
        }
        
        return string_builder_append( out, lang_serialize_token( lang, scope->token_end) );
    }

    bdc::String lang_serialize_token(const Language& lang, const Token& token)
    {
        if ( token.type == Token_Type_NULL )
            return {};

        return token.view();
    }

    bdc::String_Builder& lang_serialize_graph(const Language& lang, bdc::String_Builder& out, const Graph* graph )
    {
        const Node* root_node = graph_root(graph);
        if ( root_node == nullptr )
        {
            TOOLS_LOG(Verbosity_Error, "Serializer", "a root primary_child is expected to serialize the graph\n");
            return out;
        }
        return lang_serialize_node(lang, out, root_node, Serialization_Flag_RECURSE);
    }

    bdc::String lang_serialize_bool(const Language& lang, bool b)
    {
        return b ? "true" : "false";
    }

    bdc::String lang_serialize_int(const Language& lang, int i)
    {
        return string_printf( "%i", i );
    }

    bdc::String lang_serialize_double(const Language& lang, double d)
    {
        return string_printf( "%d", d );
    }

    bdc::String_Builder& lang_serialize_for_loop(const Language& lang, bdc::String_Builder& out, const Node* _for_loop)
    {
        ASSERT( _for_loop->type == Node_Type_FOR_LOOP );

        string_builder_append( out, lang_serialize_token( lang, _for_loop->component.branching.branch_prefix) );
        string_builder_append( out, lang_serialize_token_type_default(lang, Token_Type_parenthesis_open) );
        {
            const Node_Slot* init_slot = node_find_slot_by_property_name(_for_loop, INITIALIZATION_PROPERTY, Node_Slot::Flag_INPUT );
            const Node_Slot* cond_slot = node_find_slot_by_property_name(_for_loop, CONDITION_PROPERTY, Node_Slot::Flag_INPUT );
            const Node_Slot* iter_slot = node_find_slot_by_property_name(_for_loop, ITERATION_PROPERTY, Node_Slot::Flag_INPUT );
            lang_serialize_input( lang, out, init_slot, Serialization_Flag_RECURSE );
            lang_serialize_input( lang, out, cond_slot, Serialization_Flag_RECURSE );
            lang_serialize_input( lang, out, iter_slot, Serialization_Flag_RECURSE );
        }
        string_builder_append( out, lang_serialize_token_type_default(lang, Token_Type_parenthesis_close) );
        lang_serialize_node( lang, out, _for_loop->component.branching.branch_out(Branch_TRUE)->first_adjacent_node(), Serialization_Flag_RECURSE );

        return out;
    }

    bdc::String_Builder& lang_serialize_while_loop(const Language& lang, bdc::String_Builder& out, const Node* _while_loop_node)
    {
        ASSERT( _while_loop_node->type == Node_Type_WHILE_LOOP );

        // while
        String while_str = lang_serialize_token( lang, _while_loop_node->component.branching.branch_prefix);
        string_builder_append(out, while_str);

        // condition
        Serialization_Flags flags = Serialization_Flag_RECURSE
                            | Serialization_Flag_WRAP_WITH_BRACES;
        lang_serialize_input( lang, out, _while_loop_node->component.branching.condition_in(), flags );

        if ( const Node* _node = _while_loop_node->component.branching.branch_out(Branch_TRUE)->first_adjacent_node() )
        {
            lang_serialize_node( lang, out, _node, Serialization_Flag_RECURSE);
        }

        return out;
    }


    bdc::String_Builder& lang_serialize_cond_struct(const Language& lang, bdc::String_Builder& out, const Node* if_node )
    {
        ASSERT( if_node->type == Node_Type_IF_ELSE );

        // if
        String if_str = lang_serialize_token( lang, if_node->component.branching.branch_prefix );
        string_builder_append(out,  if_str );

        // condition
        Serialization_Flags flags = Serialization_Flag_RECURSE
                            | Serialization_Flag_WRAP_WITH_BRACES;
        lang_serialize_input(lang, out, if_node->component.branching.condition_in(), flags );

        // when condition is true
        lang_serialize_node(lang, out, if_node->component.branching.branch_out(Branch_TRUE)->first_adjacent_node(), Serialization_Flag_RECURSE );

        // when condition is false
        string_builder_append(out, lang_serialize_token( lang, if_node->component.branching.branch_suffix) );
        lang_serialize_node(lang, out, if_node->component.branching.branch_out(Branch_FALSE)->first_adjacent_node(), Serialization_Flag_RECURSE );

        return out;
    }

    // Language definition ------------------------------------------------------------------------------------------------------------

    bdc::String_Builder& lang_serialize_property(const Language& lang, bdc::String_Builder& out, const Node_Property* property)
    {
        const String property_str = lang_serialize_token( lang, property->token);
        return string_builder_append( out, property_str );
    }

    const Operator* lang_find_operator(const Language& lang, const Operator& op)
    {
        auto found = std::find(lang.operators.cbegin(), lang.operators.cend(), op );

        if (found != lang.operators.end())
            return &*found;

        return nullptr;
    }

    bool lang_is_operator(const Language& lang, const Type_Descriptor* type)
    {
        switch ( type->function.args.size )
        {
            case 1:     return lang_find_operator( lang, Operator{ type->name, Operator_Type::Unary} );
            case 2:     return lang_find_operator( lang, Operator{ type->name, Operator_Type::Binary} );
            default:    return false;
        }
    }

    bdc::String lang_serialize_token_type_default(const Language& lang, Token_Type _token_t)
    {
        switch (_token_t)
        {
            case Token_Type_end_of_line:     return "\n"; // TODO: handle all platforms
            case Token_Type_operator:        return "operator";
            case Token_Type_identifier:      return "identifier";
            case Token_Type_literal_string:  return "\"\"";
            case Token_Type_literal_double:  return "0.0";
            case Token_Type_literal_int:     return "0";
            case Token_Type_literal_bool:    return "false";
            case Token_Type_literal_any:     return "0";
            case Token_Type_NULL:          [[fallthrough]];
            case Token_Type_literal_unknown: return "";
            default:
            {
                {
                    auto found = lang.keyword_by_token_type.find(_token_t);
                    if (found != lang.keyword_by_token_type.cend())
                    {
                        return found->second;
                    }
                }
                {
                    auto found = lang.single_char_by_keyword.find(_token_t);
                    if (found != lang.single_char_by_keyword.cend())
                    {
                        return String{found->second};
                    }
                }
                return "<?>";
            }
        }
    }

    bdc::String lang_serialize_type(const Language& lang, const Type_Descriptor* type)
    {
        auto found = lang.keyword_by_type_id.find( type->id );
        if (found != lang.keyword_by_type_id.cend())
        {
            return found->second;
        }
        return "";
    }

    int lang_get_precedence(const Language& lang, const Type_Descriptor* _func_type)
    {
        if (!_func_type)
            return std::numeric_limits<int>::min(); // default

        Operator expected_operator{ _func_type->name, static_cast<Operator_Type>(_func_type->function.args.size) };

        if (const Operator* found_operator = lang_find_operator(lang, expected_operator))
            return found_operator->precedence;
        return std::numeric_limits<int>::max();
    }

    const Type_Descriptor* lang_get_type(const Language& lang, Token_Type _token)
    {
        auto found = lang.type_descriptor_by_token_type.find(_token);
        if ( found != lang.type_descriptor_by_token_type.end() )
            return found->second;
        return nullptr;
    }

    bool _lang_accepts_suffix(const Language& lang, Token_Type type)
    {
        return type != Token_Type_identifier          // identifiers must stay clean because they are reused
                && type != Token_Type_parenthesis_open    // ")" are lost when creating AST
                && type != Token_Type_parenthesis_close;  // "(" are lost when creating AST
    }

    Token_Type lang_type_to_literal_token_type(const Language& lang, const Type_Descriptor *type)
    {
        if (type == type_get<double>() )
            return Token_Type_literal_double;
        if (type == type_get<i16_t>() )
            return Token_Type_literal_int;
        if (type == type_get<int>() )
            return Token_Type_literal_int;
        if (type == type_get<bool>() )
            return Token_Type_literal_bool;
        if (type == type_get<bdc::String>() )
            return Token_Type_literal_string;
        if (type == type_get<any>() )
            return Token_Type_literal_any;
        return Token_Type_literal_unknown;
    }

    Node* lang_parse_atomic_code_block(Language& lang, Scope* parent_scope, Node_Slot* flow_out)
    {
        TOOLS_DEBUG_LOG(Verbosity_Diagnostic, "Parser", "Parsing atomic code block ..\n");
        ASSERT(flow_out);

        // most common case
        Node* block = nullptr;
             if ( (block = lang_parse_scoped_block(lang, parent_scope, flow_out)) );
        else if ( (block = lang_parse_return(lang, parent_scope, flow_out)));
        else if ( (block = lang_parse_expression_block(lang, parent_scope, flow_out)) );
        else if ( (block = lang_parse_if_block(lang, parent_scope, flow_out)) );
        else if ( (block = lang_parse_for_block(lang, parent_scope, flow_out)) );
        else if ( (block = lang_parse_while_block(lang, parent_scope, flow_out)) ) ;
        else      (block = lang_parse_empty_block(lang, parent_scope, flow_out));

        if ( block )
        {
            if ( Token tok = lang.ribbon.eat_if(Token_Type_end_of_instruction) )
            {
                block->suffix = tok;
            }

            TOOLS_DEBUG_LOG(Verbosity_Diagnostic, "Parser", TOOLS_OK " Block found (class \"%s\")\n", block->get_class()->name.c_str() );
            return block;
        }

        TOOLS_DEBUG_LOG(Verbosity_Diagnostic, "Parser", TOOLS_KO " No block found\n");
        return nullptr;
    }

    bdc::String_Builder& lang_serialize_literal(const Language& lang, bdc::String_Builder& out, const Node* node)
    {
        ASSERT( node->type == Node_Type_LITERAL );
        return lang_serialize_property( lang, out, node->value );
    }

    bdc::String_Builder& lang_serialize_empty_instruction(const Language& lang, bdc::String_Builder& out, const Node* node)
    {
        ASSERT( node->type == Node_Type_EMPTY_INSTRUCTION );
        return string_builder_append( out, lang_serialize_token( lang, node->value->token ) );
    }

    Node* lang_parse_empty_block(Language& lang, Scope* parent_scope, Node_Slot* flow_out)
    {
        if ( lang.ribbon.peek(Token_Type_end_of_instruction) )
        {
            Node* node = graph_create_empty_instruction( lang.graph, parent_scope );
            graph_connect( flow_out, node->flow_in(), Graph_Flag_ALLOW_SIDE_EFFECTS);
            return node;
        }
        return nullptr;
    }

    void lang_reset_graph(Language& lang, Graph* new_graph)
    {
        lang.graph = new_graph; // memory not owned
    }
} // namespace ndbl