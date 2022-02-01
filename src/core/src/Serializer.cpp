#include <nodable/Serializer.h>
#include <nodable/Member.h>
#include <nodable/InvokableComponent.h>
#include <nodable/GraphNode.h>
#include <nodable/Node.h>
#include <nodable/VariableNode.h>
#include <nodable/ScopeNode.h>
#include <nodable/InstructionNode.h>
#include <nodable/ConditionalStructNode.h>
#include <nodable/ForLoopNode.h>

using namespace Nodable;

std::string& Serializer::serialize(std::string& _result, const InvokableComponent *_component)const
{

    const Invokable* invokable = _component->get_invokable();

    if ( invokable->get_invokable_type() == Invokable::Type::Function )
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
            auto l_handed_operator = _component->get_owner()->getConnectedOperator(args[0]);
            auto r_handed_operator = _component->get_owner()->getConnectedOperator(args[1]);
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
            auto inner_operator = _component->get_owner()->getConnectedOperator(args[0]);

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

std::string& Serializer::serialize(std::string &_result, const Type& _type) const
{
    return _result.append(language->getSemantic()->type_to_string(_type) );
}

std::string& Serializer::serialize(std::string& _result, const VariableNode* _node) const
{
    // type
    serialize(_result, _node->getTypeToken() );

    // var name
    auto identifierTok = _node->getIdentifierToken();
    if ( identifierTok ) _result.append( identifierTok->m_prefix);
    _result.append( _node->getName());
    if ( identifierTok ) _result.append( _node->getIdentifierToken()->m_suffix);

    // assigment ?
    if ( _node->getAssignmentOperatorToken() )
    {
        Member* value = _node->value();

        _result.append(_node->getAssignmentOperatorToken()->m_prefix );
        _result.append(_node->getAssignmentOperatorToken()->m_word );
        _result.append(_node->getAssignmentOperatorToken()->m_suffix );

        if ( value->hasInputConnected() )
        {
            serialize(_result, value);
        }
        else
        {
            _result.append( value->getSourceToken()->m_prefix);
            serialize(_result, _node->value()->getData());
            _result.append( value->getSourceToken()->m_suffix);
        }

    }
    return _result;
}

std::string& Serializer::serialize(std::string& _result, const Variant* variant) const
{
    if (variant->isType(Type_String))
    {
        return _result.append('"' + (std::string)*variant + '"');
    }
    else
    {
        return _result.append(variant->convert_to<std::string>() );
    }
}

std::string& Serializer::serialize(std::string& _result, const Member * _member, bool followConnections) const
{
    const Token *sourceToken = _member->getSourceToken();
    if (sourceToken)
    {
        _result.append(sourceToken->m_prefix);
    }

    auto owner = _member->getOwner();
    if ( followConnections && owner && _member->allowsConnection(Way_In) && owner->hasWireConnectedTo(_member) )
    {
        Member*           sourceMember      = owner->getSourceMemberOf(_member);
        InvokableComponent* compute_component = sourceMember->getOwner()->getComponent<InvokableComponent>();

        if ( compute_component )
        {
            serialize(_result, compute_component );
        }
        else
        {
            serialize(_result, sourceMember, false);
        }
    }
    else
    {
        if (owner && owner->get_class() == VariableNode::Get_class())
        {
            auto variable = owner->as<VariableNode>();
            _result.append(variable->getName() );
        }
        else
        {
            serialize(_result, _member->getData() );
        }
    }

    if (sourceToken)
    {
        _result.append(sourceToken->m_suffix);
    }
    return _result;
}

std::string& Serializer::serialize(std::string& _result, const ScopeNode* _block) const
{

    serialize( _result, _block->get_begin_scope_token() );

    if (!_block->get_children().empty())
    {
        for( auto& eachChild : _block->get_children() )
        {
            auto clss = eachChild->get_class();
            if ( clss->is<InstructionNode>())
            {
                serialize(_result, eachChild->as<InstructionNode>());
            }
            else if ( clss->is<ScopeNode>() )
            {
                serialize( _result, eachChild->as<ScopeNode>());
            }
            else if ( clss->is<ConditionalStructNode>() )
            {
                 serialize( _result, eachChild->as<ConditionalStructNode>());
            }
            else if ( clss->is<ForLoopNode>() )
            {
                 serialize( _result, eachChild->as<ForLoopNode>());
            }
            else if ( clss->is<ScopeNode>() )
            {
                serialize( _result, eachChild->as<ScopeNode>());
            }
            else
            {
                NODABLE_ASSERT(false); // Node class not handled !
            }
        }
    }

    serialize( _result, _block->get_end_scope_token() );

    return _result;
}

std::string& Serializer::serialize(std::string& _result, const InstructionNode* _instruction ) const
{
    const Member* value = _instruction->value();

    if ( value->hasInputConnected() )
    {
        // var declaration ?
        if ( auto variableNode = value->getInput()->getOwner()->as<VariableNode>() )
        {
            serialize( _result, variableNode );
        }
        else
        {
            serialize( _result, value );
        }
    }
    else
    {
        serialize( _result, value );
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

    Member* input = _for_loop->get_init_expr()->getInput();
    if ( input && input->getOwner()->get_class()->is<VariableNode>() )
    {
        serialize( _result, input->getOwner()->as<VariableNode>() );
    }
    else
    {
        serialize( _result, _for_loop->get_init_expr() );
    }

    serialize( _result, TokenType_EndOfInstruction );
    serialize( _result, _for_loop->get_condition() );
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
    serialize( _result, _condStruct->get_condition() );
    serialize( _result, TokenType_CloseBracket );

    // if scope
    if ( auto* ifScope = _condStruct->get_condition_true_branch() )
        serialize( _result, ifScope );

    // else & else scope
    if ( const Token* tokenElse = _condStruct->get_token_else() )
    {
        serialize( _result, tokenElse );
        Node* elseScope = _condStruct->get_children()[1];
        Reflect::Class* elseClass = elseScope->get_class();
        if ( elseClass == ConditionalStructNode::Get_class()) // else if ?
        {
            this->serialize( _result, elseScope->as<ConditionalStructNode>() );
        }
        else
        {
            this->serialize( _result, elseScope->as<ScopeNode>() );
        }
    }
    return _result;
}
