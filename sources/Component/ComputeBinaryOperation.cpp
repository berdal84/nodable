#include "ComputeBinaryOperation.h"
#include "Log.h"		// for LOG_DEBUG(...)
#include "Member.h"
#include "Variable.h"
#include "Language.h"

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
	auto needParentheses = [&](Member * _input)->bool
	{
		if (_input != nullptr )
		{
			auto node = _input->getOwner()->as<Node>();

			if (node->hasComponent<ComputeBinaryOperation>())
			{

				if (auto leftOperationComponent = node->getComponent<ComputeBinaryOperation>())
				{
					if (language->needsToBeEvaluatedFirst(ope, leftOperationComponent->ope))
						return true;
				}
			}
		}
		return false;
	};

	std::string expr;

	// Left part of the expression
	bool leftExpressionNeedsParentheses  = needParentheses(this->args[0]->getInputMember());
	if (leftExpressionNeedsParentheses) expr.append("( ");
	expr.append( this->args[0]->getSourceExpression() );
	if (leftExpressionNeedsParentheses) expr.append(" )");

	// Operator
	expr.append( " " );
	expr.append( this->ope->identifier );
	expr.append( " " );

	// Right part of the expression
	bool rightExpressionNeedsParentheses = needParentheses(this->args[1]->getInputMember());
	if (rightExpressionNeedsParentheses) expr.append("( ");
	expr.append(this->args[1]->getSourceExpression());
	if (rightExpressionNeedsParentheses) expr.append(" )");

	// Apply the new string to the result's source expression.
	this->result->setSourceExpression(expr.c_str());
}
