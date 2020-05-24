#include "BinaryOperation.h"
#include "Log.h"		// for LOG_DEBUG(...)
#include "Member.h"
#include "Variable.h"
#include "Language.h"

using namespace Nodable;

 // BinaryOperationComponent :
//////////////////////////
void BinaryOperationComponent::setLeft(Member* _value){
	left = _value;	
};

void BinaryOperationComponent::setRight(Member* _value) {
	right = _value;
};

void BinaryOperationComponent::updateResultSourceExpression()const
{
	/*
		Labmda funtion to check if parentheses are needed for the expression of the inputMember speficied as parameter.
	*/
	auto needParentheses = [&](Member * _input)->bool
	{
		if (_input != nullptr )
		{
			auto node = _input->getOwner()->as<Node>();

			if (node->hasComponent<BinaryOperationComponent>())
			{

				if (auto leftOperationComponent = node->getComponent<BinaryOperationComponent>())
				{
					auto leftOperatorString = leftOperationComponent->getOperatorAsString();

					if (language->needsToBeEvaluatedFirst(operatorAsString, leftOperatorString))
						return true;
				}
			}
		}
		return false;
	};

	std::string expr;

	// Left part of the expression
	bool leftExpressionNeedsParentheses  = needParentheses(this->left->getInputMember());
	if (leftExpressionNeedsParentheses) expr.append("( ");
	expr.append( this->left->getSourceExpression() );
	if (leftExpressionNeedsParentheses) expr.append(" )");

	// Operator
	expr.append( " " );
	expr.append( this->operatorAsString );
	expr.append( " " );

	// Right part of the expression
	bool rightExpressionNeedsParentheses = needParentheses(this->right->getInputMember());
	if (rightExpressionNeedsParentheses) expr.append("( ");
	expr.append(this->right->getSourceExpression());
	if (rightExpressionNeedsParentheses) expr.append(" )");

	// Apply the new string to the result's source expression.
	this->result->setSourceExpression(expr.c_str());
}

 // Node_Add :
//////////////

bool Add::update()
{
	if (!left->isType(Type_Unknown) && !right->isType(Type_Unknown)) {
		switch (left->getType())
		{
			case Type_String:
			{
				auto sum = (std::string) * left + (std::string) * right;
				result->set(sum);
				break;
			}

			case Type_Boolean:
			{
				auto sum = (bool)*left || (bool)*right;
				result->set(sum);
				break;
			}

			default:
			case Type_Number:
			{
				auto sum = (double)*left + (double)*right;
				result->set(sum);
				break;
			}
		}
	} else {
		result->unset();
	}

	updateResultSourceExpression();

	return true;
}

 // Node_Substract :
///////////////////////

bool Subtract::update()
{
	if (!left->isType(Type_Unknown) && !right->isType(Type_Unknown)) {
		double sub = (double)*left - (double)*right;
		result->set(sub);
	}
	else {
		result->unset();
	}

	updateResultSourceExpression();

	return true;
}

 // Node_Divide :
///////////////////////

bool Divide::update()
{
	if (!left->isType(Type_Unknown) && !right->isType(Type_Unknown) && (double)*right != 0.0f) {
		auto div = (double)*left / (double)*right;
		result->set(div);

	} else {
		result->unset();
	}

	updateResultSourceExpression();

	return true;
}

 // Node_Multiply :
///////////////////////

bool Multiply::update()
{
	if (!left->isType(Type_Unknown) && !right->isType(Type_Unknown)) {
		switch (left->getType())
		{
		case Type_Boolean:
		{
			auto mul = (bool)*left && (bool)*right;
			result->set(mul);
			break;
		}

		default:
		{
			auto mul = (double)*left * (double)*right;
			result->set(mul);
			break;
		}
		}
	} else {
		result->unset();
	}

	
	updateResultSourceExpression();

	return true;
}

 // Node_Assign :
///////////////////////

bool Assign::update()
{
	switch (right->getType())
	{
		case Type_Number:
		default:
		{
			auto v = (double)*right;
			result->set(v);
			left->set(v);
			break;
		}
		case Type_String:
		{
			auto v = (std::string)*right;
			result->set(v);
			left->set(v);
			break;
		}

	}
	
	updateResultSourceExpression();

	return true;
}

MultipleArgFunctionComponent::MultipleArgFunctionComponent(FunctionPrototype _prototype, const Language* _language):
	FunctionComponent(_language),
	prototype(_prototype)
{
	size_t i = 0;
	while (args.size() < _prototype.getArgs().size())
		args.push_back(nullptr);
}

bool MultipleArgFunctionComponent::update()
{

	if (prototype.call == NULL) {
		LOG_ERROR("Unable to find %s's nativeFunction.\n", prototype.getIdentifier().c_str());
		return false;
	}

	if (prototype.call(result, args))
		LOG_ERROR("Evaluation of %s's native function failed !\n", prototype.getIdentifier().c_str());
	else
		this->updateResultSourceExpression();

	return true;
}

void MultipleArgFunctionComponent::updateResultSourceExpression() const
{
	std::string expr = language->serializeFunction(prototype, args);
	this->result->setSourceExpression(expr.c_str());
}
