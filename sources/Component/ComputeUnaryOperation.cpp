#include "ComputeUnaryOperation.h"
#include "Member.h"
#include "Variable.h"
#include "Language/Common/Language.h"

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
