#include <nodable/core/Wire.h>

using namespace ndbl;

void Wire::sanitize()
{
    NDBL_EXPECT(members.src != nullptr, "member.src is nullptr");
    NDBL_EXPECT(members.dst != nullptr, "member.dst is nullptr");
    NDBL_EXPECT(members.src != members.dst, "member.src and members.dst are identical");
}
