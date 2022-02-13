#include <nodable/Serializer.h>
#include <nodable/Member.h>
#include <nodable/InvokableComponent.h>
#include <nodable/GraphNode.h>
#include <nodable/VariableNode.h>
#include <nodable/InstructionNode.h>
#include <nodable/ConditionalStructNode.h>
#include <nodable/ForLoopNode.h>
#include <nodable/Scope.h>
#include <nodable/LiteralNode.h>

using namespace Nodable;

std::string& Serializer::serialize(std::string& _result, const InvokableComponent *_component)const
{

    const IInvokable* invokable = _component->get_invokable();

    if (invokable->get_invokable_type() == IInvokable::Type::Function )
    {
        serialize(_result, _component->get_invokable()->get_signature(), _component->get_args());
    }
    else
    {
        // generic serialize member lambda
        auto serialize_member_with_or_without_brackets = [this, &_result](Member* member, bool needs_brackets)
        {
            if (needs_brackets)
            {
                serialize(_result, TokenType_OpenBracket);
            }

            serialize(_result, member);

            if (needs_brackets)
            {
                serialize(_result, TokenType_CloseBracket);
            }
        };

        auto ope = reinterpret_cast<const InvokableOperator*>(invokable);
        std::vector<Member *> args = _component->get_args();

        if (ope->get_operator_type() == InvokableOperator::Type::Binary )
        {
            // Get the left and right source operator
            auto l_handed_operator = _component->get_owner()->get_connected_operator(args[0]);
            auto r_handed_operator = _component->get_owner()->get_connected_operator(args[1]);
            // Left part of the expression
            {
                // TODO: check parsed brackets for prefix/suffix
                bool needs_brackets = l_handed_operator &&
                                    !language->hasHigherPrecedenceThan(l_handed_operator, ope );

                serialize_member_with_or_without_brackets(args[0], needs_brackets);
            }

            // Operator
            const Token *sourceToken = _component->get_source_token();
            if (sourceToken) {
                _result.append(sourceToken->m_prefix);
            }
            _result.append(ope->get_short_identifier());
            if (sourceToken) {
                _result.append(sourceToken->m_suffix);
            }

            // Right part of the expression
            {
                // TODO: check parsed brackets for prefix/suffix
                bool needs_brackets = r_handed_operator
                                    && (r_handed_operator->get_operator_type() == InvokableOperator::Type::Unary
                                        || !language->hasHigherPrecedenceThan( r_handed_operator, ope )
                                    );

                serialize_member_with_or_without_brackets(args[1], needs_brackets);
            }
        }
        else if (ope->get_operator_type() == InvokableOperator::Type::Unary )
        {
            auto inner_operator = _component->get_owner()->get_connected_operator(args[0]);

            // operator ( ... innerOperator ... )   ex:   -(a+b)

            // Operator
            const Token *sourceToken = _component->get_source_token();

            if (sourceToken) {
                _result.append(sourceToken->m_prefix);
            }

            _result.append(ope->get_short_identifier());

            if (sourceToken) {
                _result.append(sourceToken->m_suffix);
            }

            bool needs_brackets = inner_operator;
            serialize_member_with_or_without_brackets(args[0], needs_brackets);
        }
    }
    return _result;
}

std::string& Serializer::serialize(std::string& _result, const FunctionSignature*   _signature, const std::vector<Member*>& _args) const
{
    _result.append(_signature->get_identifier());
    serialize(_result, TokenType_OpenBracket);

    for (auto it = _args.begin(); it != _args.end(); it++) {
        serialize(_result, *it);

        if (*it != _args.back()) {
            serialize(_result, TokenType_Separator);
        }
    }

    serialize(_result, TokenType_CloseBracket);
    return _result;
}

std::string& Serializer::serialize(std::string& _result, const FunctionSignature* _signature) const {

    serialize(_result, _signature->get_return_type());
    _result.append(" ");
    _result.append(_signature->get_identifier() );
    serialize(_result, TokenType_OpenBracket);

    auto args = _signature->get_args();
    for (auto it = args.begin(); it != args.end(); it++)
    {
        if (it != args.begin())
        {
            serialize( _result, TokenType_Separator);
            _result.append(" ");
        }
        serialize(_result, it->m_type);
    }

    serialize(_result, TokenType_CloseBracket );
    return  _result;
}

std::string& Serializer::serialize(std::string& _result, const TokenType& _type) const
{
    return _result.append(language->getSemantic()->token_type_to_string(_type) );
}

std::string& Serializer::serialize(std::string &_result, const Reflect::Type& _type) const
{
    return _result.append(language->getSemantic()->type_to_string(_type) );
}

std::string& Serializer::serialize(std::string& _result, const VariableNode* _node) const
{
    // type
    if ( const Token* type_tok = _node->get_type_token() )
    {
        serialize(_result, type_tok );
    }
    else // in case no token found (means was not parsed but created by the user)
    {
        serialize(_result, _node->get_value()->get_type() );
        _result.append(" ");
    }

    // var name
    auto identifierTok = _node->get_identifier_token();
    if ( identifierTok ) _result.append( identifierTok->m_prefix);
    _result.append(_node->get_name());
    if ( identifierTok ) _result.append(_node->get_identifier_token()->m_suffix);

    // assigment ?
    if ( const Token* assign_tok = _node->get_assignment_operator_token() )
    {
        Member* value = _node->get_value();

        _result.append(assign_tok->m_prefix );
        _result.append(assign_tok->m_word );
        _result.append(assign_tok->m_suffix );

        if (value->has_input_connected() )
        {
            serialize(_result, value);
        }
        else
        {
            _result.append(value->get_src_token()->m_prefix);
            serialize(_result, _node->get_value()->get_data());
            _result.append(value->get_src_token()->m_suffix);
        }

    }
    return _result;
}

std::string& Serializer::serialize(std::string& _result, const Variant* variant) const
{
    if (variant->is(Reflect::Type_String))
    {
        return _result.append('"' + variant->convert_to<std::string>() + '"');
    }
    else
    {
        return _result.append(variant->convert_to<std::string>() );
    }
}

std::string& Serializer::serialize(std::string& _result, const Member * _member, bool followConnections) const
{
    const Token *sourceToken = _member->get_src_token();
    if (sourceToken)
    {
        _result.append(sourceToken->m_prefix);
    }

    auto owner = _member->get_owner();
    if (followConnections && owner && _member->allows_connection(Way_In) && owner->has_wire_connected_to(_member) )
    {
        Member* src_member = _member->get_input();
        InvokableComponent* compute_component = src_member->get_owner()->get<InvokableComponent>();

        if ( compute_component )
        {
            serialize(_result, compute_component );
        }
        else
        {
            serialize(_result, src_member, false);
        }
    }
    else
    {
        if (owner && owner->get_class() == VariableNode::Get_class())
        {
            auto variable = owner->as<VariableNode>();
            _result.append(variable->get_name() );
        }
        else
        {
            serialize(_result, _member->get_data() );
        }
    }

    if (sourceToken)
    {
        _result.append(sourceToken->m_suffix);
    }
    return _result;
}

std::string& Serializer::serialize(std::string& _result, const Node* _node) const
{
    NODABLE_ASSERT(_node != nullptr)
    auto clss = _node->get_class();

    if ( clss->is<InstructionNode>())
    {
        serialize(_result, _node->as<InstructionNode>());
    }
    else if ( clss->is<ConditionalStructNode>() )
    {
        serialize( _result, _node->as<ConditionalStructNode>());
    }
    else if ( clss->is<ForLoopNode>() )
    {
        serialize( _result, _node->as<ForLoopNode>());
    }
    else if ( _node->has<Scope>() )
    {
        serialize( _result, _node->get<Scope>() );
    }
    else if ( _node->is<LiteralNode>() )
    {
        serialize( _result, _node->as<LiteralNode>()->get_value() );
    }
    else if ( _node->is<VariableNode>() )
    {
        serialize( _result, _node->as<VariableNode>() );
    }
    else if ( _node->has<InvokableComponent>() )
    {
        serialize( _result, _node->get<InvokableComponent>() );
    }
    else
    {
        std::string message = "Unable to serialize ";
        message.append( _node->get_class()->get_name() );
        throw std::runtime_error( message );
    }

    return _result;
}

std::string& Serializer::serialize(std::string& _result, const Scope* _scope) const
{

    serialize(_result, _scope->get_begin_scope_token() );
    auto& children = _scope->get_owner()->children_slots();
    if (!children.empty())
    {
        for( auto& eachChild : children )
        {
            serialize( _result, eachChild );
        }
    }

    serialize(_result, _scope->get_end_scope_token() );

    return _result;
}

std::string& Serializer::serialize(std::string& _result, const InstructionNode* _instruction ) const
{
    const Member* root_node_member = _instruction->get_root_node_member();

    if (root_node_member->has_input_connected() && root_node_member->is_defined() )
    {
        const Node* root_node = (const Node*)*root_node_member;
        serialize( _result, root_node );
    }

    return serialize( _result, _instruction->end_of_instr_token() );
}

std::string& Serializer::serialize(std::string& _result, const Token* _token)const
{
    if ( _token )
    {
        _result.append( _token->m_prefix);
        serialize( _result, _token->m_type );
        _result.append( _token->m_suffix);
    }
    return _result;
}

std::string& Serializer::serialize(std::string& _result, const ForLoopNode* _for_loop)const
{

    serialize( _result, _for_loop->get_token_for() );
    serialize( _result, TokenType_OpenBracket );

    // TODO: I don't like this if/else, should be implicit. Serialize Member* must do it.
    //       More work to do to know if expression is a declaration or not.

    Member* input = _for_loop->get_init_expr()->get_input();
    if ( input && input->get_owner()->get_class()->is<VariableNode>() )
    {
        serialize( _result, input->get_owner()->as<VariableNode>() );
    }
    else
    {
        serialize( _result, _for_loop->get_init_expr() );
    }

    serialize( _result, TokenType_EndOfInstruction );
    serialize( _result, _for_loop->condition_member() );
    serialize( _result, TokenType_EndOfInstruction );
    serialize( _result, _for_loop->get_iter_expr() );
    serialize( _result, TokenType_CloseBracket );

    // if scope
    if ( auto* scope = _for_loop->get_condition_true_branch() )
    {
        serialize( _result, scope );
    }

    return _result;
}

std::string& Serializer::serialize(std::string& _result, const ConditionalStructNode* _condStruct)const
{
    // if ( <condition> )
    serialize( _result, _condStruct->get_token_if() );
    serialize( _result, TokenType_OpenBracket );
    serialize( _result, _condStruct->condition_member() );
    serialize( _result, TokenType_CloseBracket );

    // if scope
    if ( auto* ifScope = _condStruct->get_condition_true_branch() )
        serialize( _result, ifScope );

    // else & else scope
    if ( const Token* tokenElse = _condStruct->get_token_else() )
    {
        serialize( _result, tokenElse );
        Scope* elseScope = _condStruct->get_condition_false_branch();
        if ( elseScope )
        {
            serialize( _result, elseScope->get_owner() );
        }
    }
    return _result;
}
