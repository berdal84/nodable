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
    auto expr   = language->serialize(this);
    result->setSourceExpression(expr.c_str());
}

