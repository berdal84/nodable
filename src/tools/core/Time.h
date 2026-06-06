#pragma once
#include <chrono>

namespace tools
{
    static int time_current_year()
    {
        const std::chrono::year_month_day ymd = std::chrono::floor<std::chrono::days>( std::chrono::system_clock::now() );
        return (int)ymd.year();
    }
}