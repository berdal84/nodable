#include "Wire.h"
#include "WireView.h"
#include "VariableNode.h"
#include <algorithm> // for std::find()

using namespace Nodable;

void Wire::setSource(Member* _source)
{
	source     = _source;
}

void Wire::setTarget(Member* _target)
{
	target     = _target;
}
