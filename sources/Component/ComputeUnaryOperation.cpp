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
	auto needParentheses = [&](Member* _input)->bool
	{
		if (_input != nullptr)
		{
			auto node = _input->getOwner()->as<Node>();

			if (node->hasComponent<ComputeBase>())
			{
				return true;
			}
		}
		return false;
	};

	std::string expr;

	// Operator
	expr.append(" ");
	expr.append(this->ope->identifier);
	expr.append(" ");

	// Left part of the expression
	bool leftExpressionNeedsParentheses = needParentheses(this->args[0]->getInputMember());
	if (leftExpressionNeedsParentheses) expr.append("( ");
	expr.append(this->args[0]->getSourceExpression());
	if (leftExpressionNeedsParentheses) expr.append(" )");

	// Apply the new string to the result's source expression.
	this->result->setSourceExpression(expr.c_str());
}
