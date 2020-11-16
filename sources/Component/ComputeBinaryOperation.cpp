#include "ComputeBinaryOperation.h"
#include "Log.h"		// for LOG_DEBUG(...)
#include "Member.h"
#include "Variable.h"
#include "Language.h"
#include "ComputeUnaryOperation.h"

using namespace Nodable;

Nodable::ComputeBinaryOperation::ComputeBinaryOperation(
	const Operator*    _operator,
	const Language*    _language):

	ComputeFunction(_operator , _language),
	ope(_operator)
{
}

void ComputeBinaryOperation::setLValue(Member* _value){
	this->args[0] = _value;	
};

void ComputeBinaryOperation::setRValue(Member* _value) {
	this->args[1] = _value;
};

void ComputeBinaryOperation::updateResultSourceExpression()const
{
	/*
		Labmda funtion to check if parentheses are needed for the expression of the inputMember speficied as parameter.
	*/
	auto getMemberSourceBinOp = [](Member * _member)-> const Operator*
	{
	    const Operator* result{};

		if (_member != nullptr )
		{
			auto node = _member->getOwner()->as<Node>();
			if (auto binOpComponent = node->getComponent<ComputeBinaryOperation>())
            {
				result = binOpComponent->ope;
            }
			else if (auto unaryOpComponent = node->getComponent<ComputeUnaryOperation>())
            {
                result = unaryOpComponent->ope;
            }
		}

		return result;
	};

	// Get the left and right source bin operator
	auto lOperator = getMemberSourceBinOp(this->args[0]->getInputMember());
	auto rOperator = getMemberSourceBinOp(this->args[1]->getInputMember());

	auto expr   = language->serializeBinaryOp(ope, args, lOperator, rOperator);

	// Apply the new string to the result's source expression.
	result->setSourceExpression(expr.c_str());
}
