#include "ComputeBinaryOperation.h"

#include <utility>
#include "Log.h"		// for LOG_DEBUG(...)
#include "Member.h"
#include "Variable.h"
#include "Language.h"

using namespace Nodable;

ComputeBinaryOperation::ComputeBinaryOperation(
	std::shared_ptr<const Operator>    _operator,
    std::shared_ptr<const Language>    _language)
	:
    ComputeFunction(std::move(_operator), std::move(_language))
{
}

void ComputeBinaryOperation::setLValue(std::shared_ptr<Member> _value){
	this->args.at(0) = std::move(_value);
};

void ComputeBinaryOperation::setRValue(std::shared_ptr<Member> _value) {
	this->args.at(1) = std::move(_value);
};

void ComputeBinaryOperation::updateResultSourceExpression()const
{
	/*
		Labmda funtion to check if parentheses are needed for the expression of the inputMember speficied as parameter.
	*/
	auto getMemberSourceBinOp = [](const std::shared_ptr<Member>& _member)-> std::shared_ptr<const Operator>
	{
		if (_member != nullptr )
		{
			auto node = std::static_pointer_cast<Node>(_member->getOwner());
			if (auto component = node->getComponent<ComputeBinaryOperation>())
				return component->ope();
		}
		return nullptr;
	};

	// Get the left and right source bin operator
	auto lBinOp = getMemberSourceBinOp(this->args.at(0)->getInputConnectedMember());
	auto rBinOp = getMemberSourceBinOp(this->args.at(1)->getInputConnectedMember());

	auto expr   = language->serializeBinaryOp(ope(), args, lBinOp, rBinOp);

	// Apply the new string to the result's source expression.
	result->setSourceExpression(expr.c_str());
}

