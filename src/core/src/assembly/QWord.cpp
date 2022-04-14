#include "nodable/core/assembly/QWord.h"
#include <string>
#include "nodable/core/String.h"

using namespace Nodable::assembly;

R_UNION_MEMBER_DEFINITION(QWord, b)
R_UNION_MEMBER_DEFINITION(QWord, u8)
R_UNION_MEMBER_DEFINITION(QWord, u16)
R_UNION_MEMBER_DEFINITION(QWord, u32)
R_UNION_MEMBER_DEFINITION(QWord, u64)
R_UNION_MEMBER_DEFINITION(QWord, i8)
R_UNION_MEMBER_DEFINITION(QWord, i16)
R_UNION_MEMBER_DEFINITION(QWord, i32)
R_UNION_MEMBER_DEFINITION(QWord, i64)
R_UNION_MEMBER_DEFINITION(QWord, f)
R_UNION_MEMBER_DEFINITION(QWord, d)
R_UNION_MEMBER_DEFINITION(QWord, ptr)

std::string QWord::to_string(const QWord& _value)
{
    return String::fmt_hex(_value.u64);
}
