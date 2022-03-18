#include <nodable/core/Wire.h>

using namespace Nodable;

void Wire::set_source(Member* _source)
{
    NODABLE_ASSERT(_source != target)
	source = _source;
}

void Wire::set_dest(Member* _target)
{
    NODABLE_ASSERT(_target != source)
	target = _target;
}
