#include "ComputeUnaryOperation.h"

#include "Log.h"		// for LOG_DEBUG(...)
#include "Member.h"
#include "Variable.h"
#include "Language.h"
#include "ComputeBinaryOperation.h"

using namespace Nodable;

ComputeUnaryOperation::ComputeUnaryOperation():ComputeFunction() {}

ComputeUnaryOperation::ComputeUnaryOperation(
	std::shared_ptr<const Operator> _operator,
    std::shared_ptr<const Language> _language)
	:
	ComputeFunction(std::move(_operator), std::move(_language))
{

}

void ComputeUnaryOperation::setLValue(std::shared_ptr<Member> _value) {
	this->args[0] = std::move(_value);
};

void ComputeUnaryOperation::updateResultSourceExpression()const
{
	/*
		Labmda funtion to check if parentheses are needed for the expression of the inputMember speficied as parameter.
	*/
	auto getMemberSourceBinOp = [](const std::shared_ptr<Member> _member)-> std::shared_ptr<const Operator>
	{
		if (_member != nullptr )
		{
			auto node = std::static_pointer_cast<Node>( _member->getOwner() ) ;
			if (auto component = node->getComponent<ComputeBinaryOperation>())
				return component->ope();
		}
		return nullptr;
	};

	// Get the inner source bin operator
	auto innerOp = getMemberSourceBinOp(this->args[0]->getInputConnectedMember());

	auto expr   = language->serializeUnaryOp(getOperator(), args, innerOp);

	// Apply the new string to the result's source expression.
	result->setSourceExpression(expr.c_str());
}

std::shared_ptr<const Operator> ComputeUnaryOperation::getOperator() const
{
    return std::static_pointer_cast<const Operator>(this->function);
}

