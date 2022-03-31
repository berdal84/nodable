#include "nodable/core/assembly/QWord.h"
#include "nodable/core/Format.h"

using namespace Nodable;

std::string assembly::QWord::to_string(const QWord& _value)
{
    return Format::fmt_hex(_value.u64);
}

