#include "ComputeBinaryOperation.h"
#include "Log.h"		// for LOG_DEBUG(...)
#include "Member.h"
#include "Variable.h"
#include "Language.h"

using namespace Nodable;

Nodable::ComputeBinaryOperation::ComputeBinaryOperation(
	const Operator*    _operator,
	const Language*    _language):

	ComputeFunction(_operator , _language)
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
		if (_member != nullptr )
		{
			auto node = _member->getOwner()->as<Node>();
			if (auto component = node->getComponent<ComputeBinaryOperation>())
				return component->ope();
		}
		return nullptr;
	};

	// Get the left and right source bin operator
	auto lBinOp = getMemberSourceBinOp(this->args[0]->getInputMember());
	auto rBinOp = getMemberSourceBinOp(this->args[1]->getInputMember());

	auto expr   = language->serializeBinaryOp(ope(), args, lBinOp, rBinOp);

	// Apply the new string to the result's source expression.
	result->setSourceExpression(expr.c_str());
}

