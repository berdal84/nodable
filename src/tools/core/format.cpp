#include "format.h"
#include <chrono>
#include <ctime>

using namespace tools;

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

string32  format::time_point_to_string(const std::chrono::system_clock::time_point &time_point)
{
    std::time_t time = std::chrono::system_clock::to_time_t(time_point);
    char buf[32] = {};
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&time));
    return {buf};
}