#include "BinaryOperation.h"
#include "Log.h"		// for LOG_DEBUG(...)
#include "Member.h"
#include "Variable.h"
#include "Language.h"

using namespace Nodable;

 // BinaryOperationComponent :
//////////////////////////

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

					if (leftOperatorString == this->operatorAsString)
						return false;

					if (NeedsToBeEvaluatedFirst(this->operatorAsString, leftOperatorString))
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

	// Righ part of the expression
	bool rightExpressionNeedsParentheses = needParentheses(this->right->getInputMember());
	if (rightExpressionNeedsParentheses) expr.append("( ");
	expr.append(this->right->getSourceExpression());
	if (rightExpressionNeedsParentheses) expr.append(" )");

	// Apply the new string to the result's source expression.
	this->result->setSourceExpression(expr.c_str());
}

/* Precendence for binary operators */
bool BinaryOperationComponent::NeedsToBeEvaluatedFirst(std::string op, std::string nextOp)
{
	auto language = Language::NODABLE;
	const bool isHigher = language->getOperatorPrecedence(op) >= language->getOperatorPrecedence(nextOp);

	return isHigher;
	
}

 // Node_Add :
//////////////

bool Add::update()
{
	switch(left->getType())
	{
		case Type_String:
		{
			auto sum = left->as<std::string>() + right->as<std::string>();
			result->set(sum);
			break;
		}

		case Type_Boolean:
		{
			auto sum = left->as<bool>() || right->as<bool>();
			result->set(sum);
			break;
		}	

		default:
		case Type_Number:
		{
			auto sum = left->as<double>() + right->as<double>();
			result->set(sum);
			break;
		}	
	}
	
	updateResultSourceExpression();

	return true;
}

 // Node_Substract :
///////////////////////

bool Substract::update()
{
	double sub = left->as<double>() - right->as<double>();
	result->set(sub);
	
	updateResultSourceExpression();

	return true;
}

 // Node_Divide :
///////////////////////

bool Divide::update()
{
	if (right->as<double>() != 0.0f)
	{
		auto div = left->as<double>() / right->as<double>();
		result->set(div);
	}

	updateResultSourceExpression();

	return true;
}

 // Node_Multiply :
///////////////////////

bool Multiply::update()
{
	switch(left->getType())
	{
		case Type_Boolean:
		{
			auto mul = left->as<bool>() && right->as<bool>();
			result->set(mul);
			break;
		}	

		default:
		{
			auto mul = left->as<double>() * right->as<double>();
			result->set(mul);
			break;
		}
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
		{
			auto v = right->as<double>();
			result->set(v);
			left->set(v);
			break;
		}
		case Type_String:
		{
			auto v = right->as<std::string>().c_str();
			result->set(v);
			left->set(v);
			break;
		}
		default:
		{
			auto v = right->as<double>();
			result->set(v);
			left->set(v);
			break;
		}
	}
	
	updateResultSourceExpression();

	return true;
}

MultipleArgFunctionComponent::MultipleArgFunctionComponent(FunctionPrototype _prototype): prototype( _prototype )
{
	size_t i = 0;
	while (args.size() < _prototype.getArgs().size())
		args.push_back(nullptr);
}

bool MultipleArgFunctionComponent::update()
{

	if (prototype.nativeFunction == NULL) {
		LOG_ERROR("Unable to find %s's nativeFunction.\n", prototype.getIdentifier().c_str());
		return false;
	}

	switch (args.size())
	{
	case 0:
		prototype.nativeFunction(result, nullptr, nullptr);
		break;
	case 1:
		prototype.nativeFunction(result, args[0], nullptr);
		break;
	case 2:
		prototype.nativeFunction(result, args[0], args[1]);
		break;
	}

	this->updateResultSourceExpression();

	return true;
}

void MultipleArgFunctionComponent::updateResultSourceExpression() const
{
	std::string expr;
	expr.append(this->prototype.getIdentifier() );
	expr.append("( ");

	for (auto it = args.begin(); it != args.end(); it++) {
		expr.append((*it)->getSourceExpression());

		if (*it != args.back()) {
			expr.append(", ");
		}
	}

	expr.append(" )");

	// Apply the new string to the result's source expression.
	this->result->setSourceExpression(expr.c_str());
}
