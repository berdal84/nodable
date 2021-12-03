#include <nodable/Serializer.h>
#include <nodable/Member.h>
#include <nodable/ComputeBinaryOperation.h>
#include <nodable/ComputeUnaryOperation.h>
#include <nodable/GraphNode.h>
#include <nodable/Node.h>
#include <nodable/VariableNode.h>
#include <nodable/CodeBlockNode.h>
#include <nodable/ScopedCodeBlockNode.h>
#include <nodable/InstructionNode.h>
#include <nodable/ConditionalStructNode.h>

using namespace Nodable;

std::string& Serializer::serialize(std::string &_result, const ComputeUnaryOperation* _operation) const
{
    auto args = _operation->getArgs();
    auto inner_operator = _operation->getOwner()->getConnectedOperator(args[0]);

    // operator ( ... innerOperator ... )   ex:   -(a+b)

    // Operator
    const Token* sourceToken = _operation->getSourceToken();
    if ( sourceToken )
    {
        _result.append(sourceToken->m_prefix);
    }

    _result.append( _operation->getOperator()->getShortIdentifier() );

    if ( sourceToken )
    {
        _result.append(sourceToken->m_suffix);
    }

    // Inner part of the expression
    {
        bool needBrackets = inner_operator;

        if (needBrackets)
        {
            serialize(_result, TokenType_OpenBracket);
        }

        serialize(_result, args[0]);

        if (needBrackets)
        {
            serialize(_result, TokenType_CloseBracket);
        }
    }
    return _result;
}

std::string& Serializer::serialize(std::string& _result, const ComputeBinaryOperation * _operation) const
{
    // Get the left and right source operator
    std::vector<Member*> args = _operation->getArgs();
    auto l_handed_operator = _operation->getOwner()->getConnectedOperator(args[0]);
    auto r_handed_operator = _operation->getOwner()->getConnectedOperator(args[1]);
    // Left part of the expression
    {
        // TODO: check parsed brackets for prefix/suffix
        bool needBrackets = l_handed_operator && !language->hasHigherPrecedenceThan(l_handed_operator, _operation->getOperator());
        if (needBrackets)
        {
            serialize(_result, TokenType_OpenBracket);
        }

        serialize(_result, args[0]);

        if (needBrackets)
        {
            serialize(_result, TokenType_CloseBracket);
        }
    }

    // Operator
    const Token* sourceToken = _operation->getSourceToken();
    if ( sourceToken )
    {
        _result.append( sourceToken->m_prefix);
    }
    _result.append( _operation->getOperator()->getShortIdentifier() );
    if ( sourceToken )
    {
        _result.append( sourceToken->m_suffix);
    }

    // Right part of the expression
    {
        // TODO: check parsed brackets for prefix/suffix
        bool needBrackets = r_handed_operator
                            && (  r_handed_operator->getType() == Operator::Type::Unary
                                  || !language->hasHigherPrecedenceThan(r_handed_operator, _operation->getOperator())
                               );

        if (needBrackets)
        {
            serialize(_result, TokenType_OpenBracket);
        }

        serialize(_result, args[1]);

        if (needBrackets)
        {
            serialize(_result, TokenType_CloseBracket);
        }
    }
    return _result;
}

std::string& Serializer::serialize(std::string& _result, const ComputeFunction *_computeFunction)const
{
    return serialize(_result, _computeFunction->getFunction()->getSignature(), _computeFunction->getArgs());
}

std::string& Serializer::serialize(std::string& _result, const ComputeBase *_operation)const
{
    if( auto computeBinOp = _operation->as<ComputeBinaryOperation>() )
    {
        return serialize(_result, computeBinOp);
    }
    else if (auto computeUnaryOp = _operation->as<ComputeUnaryOperation>() )
    {
        return serialize(_result, computeUnaryOp);
    }
    else if (auto fct = _operation->as<ComputeFunction>())
    {
        return serialize(_result, fct);
    }
    else
    {
        return _result;
    }
}

std::string& Serializer::serialize(std::string& _result, const FunctionSignature*   _signature, const std::vector<Member*>& _args) const
{
    _result.append(_signature->getIdentifier());
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

    serialize(_result, _signature->getType());
    _result.append(" ");
    _result.append( _signature->getIdentifier() );
    serialize(_result, TokenType_OpenBracket);

    auto args = _signature->getArgs();
    for (auto it = args.begin(); it != args.end(); it++)
    {
        if (it != args.begin())
        {
            serialize( _result, TokenType_Separator);
            _result.append(" ");
        }
        serialize(_result, it->type);
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
        return _result.append((std::string)*variant );
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
        auto sourceMember = owner->getSourceMemberOf(_member);

        if ( auto computeBase = sourceMember->getOwner()->getComponent<ComputeBase>() )
        {
            serialize(_result, computeBase );
        }
        else
        {
            serialize(_result, sourceMember, false);
        }
    }
    else
    {
        if (owner && owner->getClass() == VariableNode::GetClass())
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

std::string& Serializer::serialize(std::string& _result, const CodeBlockNode* _block) const
{
    if (!_block->getChildren().empty())
    {
        for( auto& eachChild : _block->getChildren() )
        {
            auto clss = eachChild->getClass();
            if ( clss == InstructionNode::GetClass())
            {
                serialize(_result, eachChild->as<InstructionNode>());
            }
            else if ( clss == ScopedCodeBlockNode::GetClass())
            {
                serialize( _result, eachChild->as<ScopedCodeBlockNode>());
            }
            else if ( clss == ConditionalStructNode::GetClass())
            {
                 serialize( _result, eachChild->as<ConditionalStructNode>());
            }
            else if ( clss == CodeBlockNode::GetClass())
            {
                serialize( _result, eachChild->as<CodeBlockNode>());
            }
            else
            {
                NODABLE_ASSERT(false); // Node class not handled !
            }
        }
    }
    return _result;
}

std::string& Serializer::serialize(std::string& _result, const InstructionNode* _instruction ) const
{
    const Member* value = _instruction->getValue();

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

    return serialize( _result, _instruction->getEndOfInstrToken() );
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

std::string& Serializer::serialize(std::string& _result, const ConditionalStructNode* _condStruct)const
{
    // if ( <condition> )
    serialize( _result, _condStruct->getTokenIf() );
    serialize( _result, TokenType_OpenBracket );
    serialize( _result, _condStruct->getCondition() );
    serialize( _result, TokenType_CloseBracket );

    // if scope
    if ( auto* ifScope = _condStruct->getBranchTrue() )
        serialize( _result, ifScope );

    // else & else scope
    if ( const Token* tokenElse = _condStruct->getTokenElse() )
    {
        serialize( _result, tokenElse );
        Node* elseScope = _condStruct->getChildren()[1];
        Reflect::Class* elseClass = elseScope->getClass();
        if ( elseClass == ConditionalStructNode::GetClass()) // else if ?
        {
            this->serialize( _result, elseScope->as<ConditionalStructNode>() );
        }
        else
        {
            this->serialize( _result, elseScope->as<ScopedCodeBlockNode>() );
        }
    }
    return _result;
}

std::string& Serializer::serialize(std::string& _result, const ScopedCodeBlockNode* _scope)const
{
    if ( _scope != nullptr )
    {
        NODABLE_ASSERT(_scope->getClass() == ScopedCodeBlockNode::GetClass());

        serialize( _result, _scope->getBeginScopeToken() );

        for (auto eachChild : _scope->getChildren())
        {
            Reflect::Class* clss = eachChild->getClass();
            if (clss->isChildOf(CodeBlockNode::GetClass()))
            {
                serialize( _result, eachChild->as<CodeBlockNode>() );
            }
            else if (clss->isChildOf(InstructionNode::GetClass()))
            {
                serialize( _result, eachChild->as<InstructionNode>() );
            }
            else if (clss->isChildOf(ScopedCodeBlockNode::GetClass()))
            {
                serialize( _result, eachChild->as<ScopedCodeBlockNode>());
            }
        }

        serialize( _result, _scope->getEndScopeToken());
    }
    return _result;
}
