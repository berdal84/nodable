#include "ComputeUnaryOperation.h"
#include "Member.h"
#include "VariableNode.h"
#include "Language/Common/Language.h"

using namespace Nodable;

Nodable::ComputeUnaryOperation::ComputeUnaryOperation(
	const Operator* _operator,
	const Language* _language)
	:
	ComputeFunction(_operator, _language)
{
}

void ComputeUnaryOperation::setLValue(Member* _value) {
	this->args[0] = _value;
};

Member* ComputeUnaryOperation::getLValue() {
    return this->args[0];
};

const Operator *ComputeUnaryOperation::getOperator() const {
    return reinterpret_cast<const Operator*>(this->function);
};