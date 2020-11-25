#include "Language.h"
#include "LanguageNodable.h"
#include "Member.h"
#include <type_traits>
#include <time.h>
#include <vector>
#include <Component/ComputeBinaryOperation.h>
#include <Component/ComputeUnaryOperation.h>

#include <Node/Node.h>

using namespace Nodable;

const Language* Language::NODABLE = nullptr;

const Language* Language::Nodable() {

	if (Language::NODABLE == nullptr)
		Language::NODABLE = new LanguageNodable();

	return Language::NODABLE;
}

void Language::addOperator( Operator _operator)
{
	operators.push_back(_operator);
}

void Language::addOperator( std::string       _identifier,
                            unsigned short    _precedence,
                            FunctionSignature _prototype,
                            FunctionImplem  _implementation) {
	Operator op(_identifier, _precedence, _prototype, _implementation);
	addOperator(op);
}

bool  Language::hasHigherPrecedenceThan(const Operator* _firstOperator, const Operator* _secondOperator)const {
	return _firstOperator->precedence >= _secondOperator->precedence;
}

const Function* Nodable::Language::findFunction(const FunctionSignature& _signature) const
{
	auto predicate = [&](Function fct) {
		return fct.signature.match(_signature);
	};

	auto it = std::find_if(api.begin(), api.end(), predicate);

	if (it != api.end())
		return &*it;

	return nullptr;
}

const Operator* Language::findOperator(const std::string& _identifier) const {

	auto predicate = [&](Operator op) {
		return op.identifier == _identifier;
	};

	auto it = std::find_if(operators.cbegin(), operators.cend(), predicate);

	if (it != operators.end())
		return &*it;

	return nullptr;
}

const Operator* Language::findOperator(const FunctionSignature& _signature) const {
	
	auto predicate = [&](Operator op) {
		return op.signature.match(_signature);
	};

	auto it = std::find_if(operators.cbegin(), operators.cend(), predicate );

	if ( it != operators.end() )
		return &*it;

	return nullptr;
}


void Nodable::Language::addToAPI(Function _function)
{
	this->api.push_back(_function);
}

void Nodable::Language::addToAPI(FunctionSignature& _signature, FunctionImplem _implementation)
{
	Function f(_signature, _implementation);
	this->api.push_back(f);
}

std::string Language::serialize(const ComputeUnaryOperation* _operation) const
{
    auto args = _operation->getArgs();
    auto inner_operator = _operation->getOwner()->getConnectedOperator(args[0]);
    return serializeUnaryOp(_operation->ope, args, inner_operator);
}

std::string Language::serialize(const ComputeBinaryOperation * _operation) const
{
    // Get the left and right source operator
    std::vector<Member*> args = _operation->getArgs();
    auto l_handed_operator = _operation->getOwner()->getConnectedOperator(args[0]);
    auto r_handed_operator = _operation->getOwner()->getConnectedOperator(args[1]);

    return this->serializeBinaryOp(_operation->ope, args, l_handed_operator, r_handed_operator);
}

std::string Language::serialize(const ComputeFunction *_computeFunction)const
{
    std::string result = serialize(
            _computeFunction->getFunction()->signature,
            _computeFunction->getArgs());
    return result;
}

std::string Language::serialize(const ComputeBase *_operation)const
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
