#include "format.h"

using namespace fw;

std::string format::number(double d)
{
    std::string str {std::to_string(d)};
    limit_trailing_zeros(str, 1);
    return str;
}

std::string format::hexadecimal(u64_t _addr)
{
    char str[33];
    snprintf(str, sizeof(str), "%#llx", _addr);
    return {str};
}

std::string format::address(const void* _addr)
{
    char str[33];
    snprintf(str, sizeof(str), "%p", _addr);
    return {str};
}

void format::limit_trailing_zeros(std::string& str, int _trailing_max)
{
    // limit to _trailing_max zeros
    size_t first_zero_to_remove = str.find_last_not_of('0', str.find_last_of('.')) + 1 + _trailing_max;
    str.erase( std::min( first_zero_to_remove , std::string::npos), std::string::npos);

    // erase eventual dot
    if ( _trailing_max == 0 && str.find_last_of('.') + 1 == str.size())
    {
        str.erase(str.find_last_of('.'), std::string::npos);
    }

    if( str.back() == '.')
    {
        str.push_back('0');
    }

}

fw::string32  format::time_point_to_string(const std::chrono::system_clock::time_point &time_point)
{
    std::time_t time = std::chrono::system_clock::to_time_t(time_point);
    // The result of ctime and ctime_s is formatted like: "Www Mmm dd hh:mm:ss yyyy\n\0" (24 chars + end of line + end of string)
#ifdef WIN32
    char str[26];
    ctime_s(str, sizeof str, &time);
    return {str, 24};
#else
    return {ctime(&time), 24};
#endif
}