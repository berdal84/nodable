#include "ComputeBinaryOperation.h"
#include "Member.h"
#include "VariableNode.h"
#include "Language/Common/Language.h"

using namespace Nodable;

Nodable::ComputeBinaryOperation::ComputeBinaryOperation(
	const Operator*    _operator,
	const Language*    _language)
	:
	ComputeUnaryOperation(_operator , _language)
{
}

void ComputeBinaryOperation::setRValue(Member* _value) {
	this->args[1] = _value;
}