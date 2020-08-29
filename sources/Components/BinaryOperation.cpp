#include "BinaryOperation.h"
#include "Log.h"		// for LOG_DEBUG(...)
#include "Member.h"
#include "Variable.h"
#include "Language.h"

using namespace Nodable;

Nodable::BinOperatorComponent::BinOperatorComponent(
	const std::string& _operatorAsString,
	const Operator     _operator,
	const Language*    _language) :

	FunctionComponent(_language),
	ope(_operator),
	operatorAsString(_operatorAsString)
{
}

void BinOperatorComponent::setLeft(Member* _value){
	left = _value;	
};

void BinOperatorComponent::setRight(Member* _value) {
	right = _value;
};

void BinOperatorComponent::updateResultSourceExpression()const
{
	/*
		Labmda funtion to check if parentheses are needed for the expression of the inputMember speficied as parameter.
	*/
	auto needParentheses = [&](Member * _input)->bool
	{
		if (_input != nullptr )
		{
			auto node = _input->getOwner()->as<Node>();

			if (node->hasComponent<BinOperatorComponent>())
			{

				if (auto leftOperationComponent = node->getComponent<BinOperatorComponent>())
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

bool BinOperatorComponent::update()
{

	if (ope.implementation == NULL) {
		LOG_ERROR("Unable to find %s's nativeFunction.\n", language->serialize(ope.signature).c_str());
		return false;
	}

	std::vector<const Member*> args;
	args.push_back(left);
	args.push_back(right);

	if (ope.implementation(result, args))
		LOG_ERROR("Evaluation of %s's native function failed !\n", language->serialize(ope.signature).c_str());
	else
		this->updateResultSourceExpression();

	return true;
}


MultipleArgFunctionComponent::MultipleArgFunctionComponent(Function _function, const Language* _language):
	FunctionComponent(_language),
	function(_function)
{
	size_t i = 0;
	while (args.size() < _function.signature.getArgs().size())
		args.push_back(nullptr);
}

bool MultipleArgFunctionComponent::update()
{

	if (function.implementation == NULL) {
		LOG_ERROR("Unable to find %s's nativeFunction.\n", language->serialize(function.signature).c_str());
		return false;
	}

	if (function.implementation(result, args))
		LOG_ERROR("Evaluation of %s's native function failed !\n", language->serialize(function.signature).c_str());
	else
		this->updateResultSourceExpression();

	return true;
}

void MultipleArgFunctionComponent::updateResultSourceExpression() const
{
	std::string expr = language->serialize(function.signature, args);
	this->result->setSourceExpression(expr.c_str());
}
