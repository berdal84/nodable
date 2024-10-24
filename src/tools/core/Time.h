#pragma once
#include <ctime>

namespace tools::time
{
    inline static int current_year()
    {
        std::time_t current_time = std::time(nullptr);
        std::tm *const time_struct = std::localtime(&current_time);
        return 1900 + time_struct->tm_year;
    }
}