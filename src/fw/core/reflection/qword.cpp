#include "qword.h"
#include <string>
#include "core/format.h"

using namespace fw;

R_UNION_MEMBER_DEFINITION(qword, b)
R_UNION_MEMBER_DEFINITION(qword, u8)
R_UNION_MEMBER_DEFINITION(qword, u16)
R_UNION_MEMBER_DEFINITION(qword, u32)
R_UNION_MEMBER_DEFINITION(qword, u64)
R_UNION_MEMBER_DEFINITION(qword, i8)
R_UNION_MEMBER_DEFINITION(qword, i16)
R_UNION_MEMBER_DEFINITION(qword, i32)
R_UNION_MEMBER_DEFINITION(qword, i64)
R_UNION_MEMBER_DEFINITION(qword, f)
R_UNION_MEMBER_DEFINITION(qword, d)
R_UNION_MEMBER_DEFINITION(qword, ptr)
R_UNION_MEMBER_DEFINITION(qword, ptr_std_string)

std::string qword::to_string(const qword& _value)
{
    return format::hexadecimal(_value.u64);
}
