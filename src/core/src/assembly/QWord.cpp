#include "nodable/core/assembly/QWord.h"
#include "nodable/core/String.h"

using namespace Nodable;

std::string assembly::QWord::to_string(const QWord& _value)
{
    return String::fmt_hex(_value.u64);
}

