#include "Serializer.h"
#include "Member.h"
#include <Component/ComputeBinaryOperation.h>
#include <Component/ComputeUnaryOperation.h>
#include <Node.h>
#include <Component/Container.h>
#include <Node/Variable.h>

using namespace Nodable;

std::string Serializer::serialize(const ComputeUnaryOperation* _operation) const
{
    auto args = _operation->getArgs();
    auto inner_operator = _operation->getOwner()->getConnectedOperator(args[0]);
    return serializeUnaryOp(_operation->ope, args, inner_operator);
}

std::string Serializer::serialize(const ComputeBinaryOperation * _operation) const
{
    // Get the left and right source operator
    std::vector<Member*> args = _operation->getArgs();
    auto l_handed_operator = _operation->getOwner()->getConnectedOperator(args[0]);
    auto r_handed_operator = _operation->getOwner()->getConnectedOperator(args[1]);

    return this->serializeBinaryOp(_operation->ope, args, l_handed_operator, r_handed_operator);
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
    expr.append(serialize(TokenType::LBracket));

    for (auto it = _args.begin(); it != _args.end(); it++) {
        expr.append(serialize(*it));

        if (*it != _args.back()) {
            expr.append(serialize(TokenType::Separator));
            expr.append(serialize(TokenType::Space));
        }
    }

    expr.append(serialize(TokenType::RBracket));
    return expr;

}

std::string Serializer::serialize(const FunctionSignature& _signature) const {

    std::string result = _signature.getIdentifier() + serialize(TokenType::LBracket);
    auto args = _signature.getArgs();

    for (auto it = args.begin(); it != args.end(); it++) {

        if (it != args.begin()) {
            result.append(serialize(TokenType::Separator));
            result.append(" ");

        }
        const auto argType = (*it).type;
        result.append( serialize(argType) );

    }

    result.append( serialize(TokenType::RBracket) );

    return result;

}

std::string Serializer::serialize(const TokenType& _type) const {
    return language->dictionnary.convert(_type);
}

std::string Serializer::serializeBinaryOp(const Operator* _op, std::vector<Member*> _args, const Operator* _leftOp, const Operator* _rightOp) const
{
    std::string result;

    // Left part of the expression
    {
        bool needBrackets = _leftOp && !language->hasHigherPrecedenceThan(_leftOp, _op);
        if (needBrackets)
        {
            result.append( serialize(TokenType::LBracket));
        }

        result.append(serialize(_args[0]));

        if (needBrackets)
        {
            result.append( serialize(TokenType::RBracket));
        }
    }

    // Operator
    result.append( serialize(TokenType::Space));
    result.append(_op->identifier);
    result.append( serialize(TokenType::Space));

    // Right part of the expression
    {
        bool needBrackets = _rightOp && (  _rightOp->getType() == Operator::Type::Unary || !language->hasHigherPrecedenceThan(_rightOp, _op) );

        if (needBrackets)
        {
            result.append(serialize(TokenType::LBracket));
        }

        result.append(serialize(_args[1]));

        if (needBrackets)
        {
            result.append(serialize(TokenType::RBracket));
        }
    }

    return result;
}

std::string Serializer::serializeUnaryOp(const Operator* _op, std::vector<Member*> _args, const Operator* _innerOp) const
{
    std::string result;

    // operator ( ... innerOperator ... )   ex:   -(a+b)

    // Operator
    result.append(_op->identifier);

    // Inner part of the expression
    {
        bool needBrackets = _innerOp;

        if (needBrackets)
        {
            result.append(serialize(TokenType::LBracket));
        }

        result.append(serialize(_args[0]));

        if (needBrackets)
        {
            result.append(serialize(TokenType::RBracket));
        }
    }

    return result;
}


std::string Serializer::serialize(const Member * _member) const
{

    std::string expression;

    auto owner = _member->getOwner()->as<Node>();

    if ( owner && _member->allowsConnection(Way_In) && owner->hasWireConnectedTo(_member) )
    {
        auto sourceMember = owner->getSourceMemberOf(_member);

        if ( auto computeBase = sourceMember->getOwner()->as<Node>()->getComponent<ComputeBase>() )
        {
            expression = Serializer::serialize(computeBase);
        }
        else
        {
            expression = serialize(sourceMember);
        }

    }
    else if (owner->getClass() == mirror::GetClass<Variable>() &&
             owner->getParentContainer()->getResultVariable() != owner)
    {
        auto variable = owner->as<Variable>();
        expression = variable->getName();
    }
    else
    {

        if (_member->isType(Type::String))
        {
            expression = '"' + (std::string)*_member + '"';
        }
        else
        {
            expression = (std::string)*_member;
        }
    }

    return expression;

}