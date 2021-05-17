#include <nodable/Wire.h>

using namespace Nodable::core;

void Wire::setSource(Member* _source)
{
	source     = _source;
}

void Wire::setTarget(Member* _target)
{
	target     = _target;
}
