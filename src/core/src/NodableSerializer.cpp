#include <nodable/core/languages/NodableSerializer.h>
#include <nodable/core/Member.h>
#include <nodable/core/InvokableComponent.h>
#include <nodable/core/GraphNode.h>
#include <nodable/core/VariableNode.h>
#include <nodable/core/InstructionNode.h>
#include <nodable/core/ConditionalStructNode.h>
#include <nodable/core/ForLoopNode.h>
#include <nodable/core/Scope.h>
#include <nodable/core/reflection/func_type.h>
#include <nodable/core/LiteralNode.h>
#include <nodable/core/Operator.h>
#include <nodable/core/languages/NodableLanguage.h>

using namespace ndbl;

std::string& NodableSerializer::serialize(std::string& _out, const InvokableComponent *_component)const
{
    const func_type* type = _component->get_func_type();

    if ( _component->is_operator() )
    {
        // generic serialize member lambda
        auto serialize_member_with_or_without_brackets = [this, &_out](Member* member, bool needs_brackets)
        {
            if (needs_brackets)
            {
                serialize(_out, Token_t::fct_params_begin);
            }

            serialize(_out, member);

            if (needs_brackets)
            {
                serialize(_out, Token_t::fct_params_end);
            }
        };

        Node*            owner      = _component->get_owner();
        const MemberVec& args       = _component->get_args();
        int             precedence  = m_language.get_precedence(_component->get_function());

        switch ( type->get_arg_count() )
        {
            case 2:
            {
                // Left part of the expression
                {
                    auto l_handed_invokable = owner->get_connected_invokable(args[0]);
                    bool needs_brackets = l_handed_invokable && m_language.get_precedence(l_handed_invokable) < precedence;

                    serialize_member_with_or_without_brackets(args[0], needs_brackets);
                }

                // Operator
                std::shared_ptr<Token> sourceToken = _component->get_source_token();
                if (sourceToken)
                {
                    _out.append(sourceToken->m_prefix);
                    _out.append(sourceToken->m_word);
                    _out.append(sourceToken->m_suffix);
                }
                else
                {
                    _out.append(type->get_identifier());
                }

                // Right part of the expression
                {
                    auto r_handed_invokable = owner->get_connected_invokable(args[1]);
                    bool needs_brackets = r_handed_invokable && m_language.get_precedence(r_handed_invokable) < precedence;
                    serialize_member_with_or_without_brackets(args[1], needs_brackets);
                }
                break;
            }

            case 1:
            {
                // operator ( ... innerOperator ... )   ex:   -(a+b)

                // Operator
                std::shared_ptr<Token> token = _component->get_source_token();

                if (token) _out.append(token->m_prefix);

                _out.append(type->get_identifier());

                if (token) _out.append(token->m_suffix);

                auto inner_operator = owner->get_connected_invokable(args[0]);
                serialize_member_with_or_without_brackets(args[0], inner_operator != nullptr);
                break;
            }
        }
    }
    else
    {
        serialize(_out, type, _component->get_args());
    }

    return _out;
}

std::string& NodableSerializer::serialize(std::string& _out, const func_type*   _signature, const std::vector<Member*>& _args) const
{
    _out.append(_signature->get_identifier());
    serialize(_out, Token_t::fct_params_begin);

    for (auto it = _args.begin(); it != _args.end(); it++)
    {
        serialize(_out, *it);

        if (*it != _args.back())
        {
            serialize(_out, Token_t::fct_params_separator);
        }
    }

    serialize(_out, Token_t::fct_params_end);
    return _out;
}

std::string& NodableSerializer::serialize(std::string& _out, const func_type* _signature) const
{
    serialize(_out, _signature->get_return_type());
    _out.append(" ");
    _out.append(_signature->get_identifier() );
    serialize(_out, Token_t::fct_params_begin);

    auto args = _signature->get_args();
    for (auto it = args.begin(); it != args.end(); it++)
    {
        if (it != args.begin())
        {
            serialize( _out, Token_t::fct_params_separator);
            _out.append(" ");
        }
        serialize(_out, it->m_type);
    }

    serialize(_out, Token_t::fct_params_end );
    return  _out;
}

std::string& NodableSerializer::serialize(std::string& _out, const Token_t& _type) const
{
    return _out.append(m_language.to_string(_type) );
}

std::string& NodableSerializer::serialize(std::string &_out, type _type) const
{
    return _out.append(m_language.to_string(_type) );
}

std::string& NodableSerializer::serialize(std::string& _out, const VariableNode* _node) const
{
    const InstructionNode* decl_instr = _node->get_declaration_instr();

    // type
    if ( decl_instr )
    {
        if ( std::shared_ptr<const Token> type_tok = _node->get_type_token() )
        {
            serialize(_out, type_tok );
        }
        else // in case no token found (means was not parsed but created by the user)
        {
            serialize(_out, _node->get_value()->get_type() );
            _out.append(" ");
        }
    }

    // var name
    std::shared_ptr<const Token> identifier_token = _node->get_identifier_token();
    if ( identifier_token ) _out.append(identifier_token->m_prefix);
    _out.append(_node->get_name());
    if ( identifier_token ) _out.append(identifier_token->m_suffix);

    Member* value = _node->get_value();

    if( decl_instr && value->has_input_connected()  )
    {
        auto append_assign_tok  = [&]()
        {
            std::shared_ptr<const Token> assign_tok = _node->get_assignment_operator_token();
            if (assign_tok )
            {
                _out.append(assign_tok->m_prefix );
                _out.append(assign_tok->m_word ); // is "="
                _out.append(assign_tok->m_suffix );
            }
            else
            {
                _out.append(" = ");
            }
        };

        append_assign_tok();
        serialize(_out, value);
    }
    return _out;
}

std::string& NodableSerializer::serialize(std::string& _out, const variant* variant) const
{
    std::string variant_string = variant->convert_to<std::string>();

    if( variant->get_type() == type::get<std::string>() )
    {
        return _out.append('"' + variant_string + '"');
    }
    return _out.append(variant_string);
}

std::string& NodableSerializer::serialize(std::string& _out, const Member * _member, bool followConnections) const
{
    // specific case of a Node*
    if ( _member->get_type() == type::get<Node*>() )
    {
        if(_member->get_variant()->is_initialized())
        {
            return serialize(_out, (const Node*)*_member);
        }
    }

    std::shared_ptr<Token> sourceToken = _member->get_src_token();
    if (sourceToken)
    {
        _out.append(sourceToken->m_prefix);
    }

    auto owner = _member->get_owner();
    if (followConnections && owner && _member->allows_connection(Way_In) && owner->has_wire_connected_to(_member) )
    {
        Member* src_member = _member->get_input();
        InvokableComponent* compute_component = src_member->get_owner()->get<InvokableComponent>();

        if ( compute_component )
        {
            serialize(_out, compute_component );
        }
        else
        {
            serialize(_out, src_member, false);
        }
    }
    else
    {
        if (owner && owner->get_type() == type::get<VariableNode>() )
        {
            auto variable = owner->as<VariableNode>();
            _out.append(variable->get_name() );
        }
        else
        {
            serialize(_out, _member->get_variant() );
        }
    }

    if (sourceToken)
    {
        _out.append(sourceToken->m_suffix);
    }
    return _out;
}

std::string& NodableSerializer::serialize(std::string& _out, const Node* _node) const
{
    NODABLE_ASSERT(_node != nullptr)
    type type = _node->get_type();

    if (type.is_child_of<InstructionNode>())
    {
        serialize(_out, _node->as<InstructionNode>());
    }
    else if (type.is_child_of<ConditionalStructNode>() )
    {
        serialize( _out, _node->as<ConditionalStructNode>());
    }
    else if (type.is_child_of<ForLoopNode>() )
    {
        serialize( _out, _node->as<ForLoopNode>());
    }
    else if ( _node->has<Scope>() )
    {
        serialize( _out, _node->get<Scope>() );
    }
    else if ( _node->is<LiteralNode>() )
    {
        serialize( _out, _node->as<LiteralNode>()->get_value() );
    }
    else if ( _node->is<VariableNode>() )
    {
        serialize( _out, _node->as<VariableNode>() );
    }
    else if ( _node->has<InvokableComponent>() )
    {
        serialize( _out, _node->get<InvokableComponent>() );
    }
    else
    {
        std::string message = "Unable to serialize ";
        message.append( type.get_name() );
        throw std::runtime_error( message );
    }

    return _out;
}

std::string& NodableSerializer::serialize(std::string& _out, const Scope* _scope) const
{

    serialize(_out, _scope->get_begin_scope_token() );
    auto& children = _scope->get_owner()->children_slots();
    if (!children.empty())
    {
        for( auto& eachChild : children )
        {
            serialize( _out, eachChild );
        }
    }

    serialize(_out, _scope->get_end_scope_token() );

    return _out;
}

std::string& NodableSerializer::serialize(std::string& _out, const InstructionNode* _instruction ) const
{
    const Member* root_node_member = _instruction->get_root_node_member();

    if (root_node_member->has_input_connected() && root_node_member->get_variant()->is_initialized() )
    {
        auto root_node = (const Node*)*root_node_member;
        NODABLE_ASSERT ( root_node )
        serialize( _out, root_node );
    }

    return serialize( _out, _instruction->end_of_instr_token() );
}

std::string& NodableSerializer::serialize(std::string& _out, std::shared_ptr<const Token> _token)const
{
    if ( _token )
    {
        _out.append( _token->m_prefix);
        if ( _token->m_type == Token_t::unknown )
        {
            _out.append( _token->m_word );
        }
        else
        {
            serialize( _out, _token->m_type );
        }
        _out.append( _token->m_suffix);

    }
    return _out;
}

std::string& NodableSerializer::serialize(std::string& _out, const ForLoopNode* _for_loop)const
{

    serialize( _out, _for_loop->get_token_for() );
    serialize( _out, Token_t::fct_params_begin );

    // TODO: I don't like this if/else, should be implicit. Serialize Member* must do it.
    //       More work to do to know if expression is a declaration or not.

    Member* input = _for_loop->get_init_expr()->get_input();
    if ( input && input->get_owner()->get_type().is_child_of<VariableNode>() )
    {
        serialize( _out, input->get_owner()->as<VariableNode>() );
    }
    else
    {
        serialize( _out, _for_loop->get_init_instr() );
    }
    serialize( _out, _for_loop->get_cond_instr() );
    serialize( _out, _for_loop->get_iter_instr() );
    serialize( _out, Token_t::fct_params_end );

    // if scope
    if ( auto* scope = _for_loop->get_condition_true_scope() )
    {
        serialize( _out, scope );
    }

    return _out;
}

std::string& NodableSerializer::serialize(std::string& _out, const ConditionalStructNode* _condStruct)const
{
    // if ( <condition> )
    serialize( _out, _condStruct->get_token_if() );
    serialize( _out, Token_t::fct_params_begin );
    serialize( _out, _condStruct->get_cond_instr() );
    serialize( _out, Token_t::fct_params_end );

    // if scope
    if ( auto* ifScope = _condStruct->get_condition_true_scope() )
        serialize( _out, ifScope );

    // else & else scope
    if ( std::shared_ptr<const Token> tokenElse = _condStruct->get_token_else() )
    {
        serialize( _out, tokenElse );
        Scope* elseScope = _condStruct->get_condition_false_scope();
        if ( elseScope )
        {
            serialize( _out, elseScope->get_owner() );
        }
    }
    return _out;
}
