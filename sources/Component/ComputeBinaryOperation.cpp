#include "ComputeBinaryOperation.h"
#include "Log.h"		// for LOG_DEBUG(...)
#include "Member.h"
#include "Variable.h"
#include "Language.h"
#include "ComputeUnaryOperation.h"

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
	auto expr   = language->serialize(this);
	result->setSourceExpression(expr.c_str());
}
