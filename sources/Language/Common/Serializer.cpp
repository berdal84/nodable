#include "Serializer.h"
#include "Member.h"
#include "Component/ComputeBinaryOperation.h"
#include "Component/ComputeUnaryOperation.h"
#include "Node/GraphNode.h"
#include "Node/Node.h"
#include "Node/VariableNode.h"
#include "Node/CodeBlockNode.h"
#include "Node/ScopedCodeBlockNode.h"
#include "Node/InstructionNode.h"

using namespace Nodable;

std::string Serializer::serialize(const ComputeUnaryOperation* _operation) const
{
    std::string result;

    auto args = _operation->getArgs();
    auto inner_operator = _operation->getOwner()->getConnectedOperator(args[0]);

    // operator ( ... innerOperator ... )   ex:   -(a+b)

    // Operator
    const Token* sourceToken = _operation->getSourceToken();
    if ( sourceToken )
    {
        result.append( sourceToken->prefix);
    }

    result.append(_operation->ope->identifier);

    if ( sourceToken )
    {
        result.append( sourceToken->suffix);
    }

    // Inner part of the expression
    {
        bool needBrackets = inner_operator;

        if (needBrackets)
        {
            result.append(serialize(TokenType::OpenBracket));
        }

        result.append(serialize(args[0]));

        if (needBrackets)
        {
            result.append(serialize(TokenType::CloseBracket));
        }
    }

    return result;
}

std::string Serializer::serialize(const ComputeBinaryOperation * _operation) const
{
    std::string result;

    // Get the left and right source operator
    std::vector<Member*> args = _operation->getArgs();
    auto l_handed_operator = _operation->getOwner()->getConnectedOperator(args[0]);
    auto r_handed_operator = _operation->getOwner()->getConnectedOperator(args[1]);
    // Left part of the expression
    {
        bool needBrackets = l_handed_operator && !language->hasHigherPrecedenceThan(l_handed_operator, _operation->ope);
        if (needBrackets)
        {
            result.append( serialize(TokenType::OpenBracket));
        }

        result.append(serialize(args[0]));

        if (needBrackets)
        {
            result.append( serialize(TokenType::CloseBracket));
        }
    }

    // Operator
    const Token* sourceToken = _operation->getSourceToken();
    if ( sourceToken )
    {
        result.append( sourceToken->prefix);
    }
    result.append(_operation->ope->identifier);
    if ( sourceToken )
    {
        result.append( sourceToken->suffix);
    }

    // Right part of the expression
    {
        bool needBrackets = r_handed_operator && (  r_handed_operator->getType() == Operator::Type::Unary || !language->hasHigherPrecedenceThan(r_handed_operator, _operation->ope) );

        if (needBrackets)
        {
            result.append(serialize(TokenType::OpenBracket));
        }

        result.append(serialize(args[1]));

        if (needBrackets)
        {
            result.append(serialize(TokenType::CloseBracket));
        }
    }

    return result;
}

std::string Serializer::serialize(const ComputeFunction *_computeFunction)const
{
    std::string result = serialize(
            _computeFunction->getFunction()->signature,
            _computeFunction->getArgs());
    return result;
}

std::string Serializer::serialize(const ComputeBase *_operation)const
{

    std::string result;

    auto computeFunction = _operation->as<ComputeFunction>();

    if(auto computeBinOp = computeFunction->as<ComputeBinaryOperation>() )
    {
        result = serialize(computeBinOp);
    }
    else if (auto computeUnaryOp = computeFunction->as<ComputeUnaryOperation>() )
    {
        result = serialize(computeUnaryOp);
    }
    else
    {
        result = serialize(computeFunction);
    }

    return result;
}

std::string Serializer::serialize(
        const FunctionSignature&   _signature,
        std::vector<Member*> _args) const
{
    std::string expr;
    expr.append(_signature.getIdentifier());
    expr.append(serialize(TokenType::OpenBracket));

    for (auto it = _args.begin(); it != _args.end(); it++) {
        expr.append(serialize(*it));

        if (*it != _args.back()) {
            expr.append(serialize(TokenType::Separator));
        }
    }

    expr.append(serialize(TokenType::CloseBracket));
    return expr;

}

std::string Serializer::serialize(const FunctionSignature& _signature) const {

    std::string result = _signature.getIdentifier() + serialize(TokenType::OpenBracket);
    auto args = _signature.getArgs();

    for (auto it = args.begin(); it != args.end(); it++) {

        if (it != args.begin()) {
            result.append(serialize(TokenType::Separator));
            result.append(" ");

        }
        const auto argType = (*it).type;
        result.append( serialize(argType) );

    }

    result.append( serialize(TokenType::CloseBracket) );

    return result;

}

std::string Serializer::serialize(const TokenType& _type) const
{
    return language->getSemantic()->tokenTypeToString(_type);
}


std::string Serializer::serialize(const Member * _member) const
{

    std::string expression;

    const Token *sourceToken = _member->getSourceToken();
    if (sourceToken)
    {
        expression.append(sourceToken->prefix);
    }

    auto owner = _member->getOwner()->as<Node>();
    if ( owner && _member->allowsConnection(Way_In) && owner->hasWireConnectedTo(_member) )
    {
        auto sourceMember = owner->getSourceMemberOf(_member);

        if ( auto computeBase = sourceMember->getOwner()->as<Node>()->getComponent<ComputeBase>() )
        {
            expression.append( Serializer::serialize(computeBase) );
        }
        else
        {
            expression.append( serialize(sourceMember) );
        }

    }
    else
    {
        if (owner->getClass() == mirror::GetClass<VariableNode>())
        {
            auto variable = owner->as<VariableNode>();
            expression.append( variable->getName() );
        }
        else
        {
            if (_member->isType(Type::String))
            {
                expression.append('"' + (std::string) *_member + '"');
            }
            else
            {
                expression.append( (std::string) *_member );
            }
        }
    }

    if (sourceToken)
    {
        expression.append(sourceToken->suffix);
    }

    return expression;
}

std::string Serializer::serialize(const CodeBlockNode* _block) const
{
    if (_block->getInstructions().empty())
    {
        // TODO: implement curly brackets {} if scope explicitly has them
        return "";
    }

    std::string result;

    for( auto& eachInstruction : _block->getInstructions() )
    {
        result.append( serialize( eachInstruction->as<InstructionNode>() ) );
    }

    return result;
}

std::string Serializer::serialize(const InstructionNode* _instruction ) const
{
    std::string result;

    result.append( serialize(_instruction->value() ) );
    result.append( serialize(_instruction->endOfInstructionToken));

    return result;
}

std::string Serializer::serialize(const Token* _token)const
{
    std::string result;

    if ( _token )
    {
        result.append( _token->prefix);
        result.append( serialize(_token->type));
        result.append( _token->suffix);
    }

    return result;
}

std::string Serializer::serialize(const ScopedCodeBlockNode* _scope)const
{
    std::string result;

    for(auto eachBlock : _scope->getChildren() )
    {
        if ( eachBlock->getClass()->isChildOf(mirror::GetClass<CodeBlockNode>()) )
        {
            result.append( serialize( (const CodeBlockNode*)(eachBlock) ) );
        }
        else // is Scope for sure
        {
            result.append( serialize( (const ScopedCodeBlockNode*)(eachBlock) ) );
        }
    }

    return result;
}
