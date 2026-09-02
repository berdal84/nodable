#include "QWord.h"
#include "bdc/String.hpp"
#include "tools/core/Format.h"

using namespace tools;

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

bdc::String QWord::to_string(const QWord& _value)
{
    return Format::hexadecimal(_value.u64);
}
