#include <nodable/core/Format.h>

using namespace Nodable;

std::string Format::fmt_no_trail(double d)
{
    std::string str {std::to_string(d)};
    remove_trailing_zeros(str);
    return str;
}

std::string Format::fmt_hex(u64 _addr)
{
    char str[33];
    sprintf(str, "%#llx", _addr);
    return {str};
}

std::string Format::fmt_ptr(const void* _addr)
{
    char str[33];
    sprintf(str, "%p", _addr);
    return {str};
}

void Format::remove_trailing_zeros(std::string& str)
{
    str.erase(str.find_last_not_of('0') + 1, std::string::npos);
    if (str.find_last_of('.') + 1 == str.size())
        str.erase(str.find_last_of('.'), std::string::npos);
}

