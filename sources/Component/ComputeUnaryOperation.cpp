#include "ComputeUnaryOperation.h"

#include "Log.h"		// for LOG_DEBUG(...)
#include "Member.h"
#include "Variable.h"
#include "Language.h"
#include "ComputeBinaryOperation.h"

using namespace Nodable;

Nodable::ComputeUnaryOperation::ComputeUnaryOperation(
	const Operator* _operator,
	const Language* _language) :

	ComputeFunction(_operator, _language),
	ope(_operator)
{
}

void ComputeUnaryOperation::setLValue(Member* _value) {
	this->args[0] = _value;
};

void ComputeUnaryOperation::updateResultSourceExpression()const
{
	/*
		Labmda funtion to check if parentheses are needed for the expression of the inputMember speficied as parameter.
	*/
	auto getMemberSourceBinOp = [](Member * _member)-> const Operator*
	{
		if (_member != nullptr )
		{
			auto node = _member->getOwner()->as<Node>();
			if (auto component = node->getComponent<ComputeBinaryOperation>())
				return component->ope;
		}
		return nullptr;
	};

	// Get the inner source bin operator
	auto innerOp = getMemberSourceBinOp(this->args[0]->getInputMember());

	auto expr   = language->serializeUnaryOp(ope, args, innerOp);

	// Apply the new string to the result's source expression.
	result->setSourceExpression(expr.c_str());
}

