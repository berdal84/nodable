#include <nodable/Wire.h>

using namespace Nodable;

void Wire::setSource(Member* _source)
{
    NODABLE_ASSERT(_source != target)
	source = _source;
}

void Wire::setTarget(Member* _target)
{
    NODABLE_ASSERT(_target != source)
	target = _target;
}
