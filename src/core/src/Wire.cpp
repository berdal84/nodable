#include <nodable/core/Wire.h>

using namespace Nodable;

void Wire::sanitize()
{
    NODABLE_ASSERT(members.src != members.dst);
}
