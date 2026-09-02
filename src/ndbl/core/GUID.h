#pragma once
#include <iostream>
#include <chrono>
#include <ctime>
#include <cstdio>
#include <cstring>
#include "bdc/Allocators.hpp"
#include "bdc/String.hpp"

namespace ndbl
{
    using namespace bdc;

    static u64_t g_unique_id_generator_counter = 0;

    // 
    // Generates a unique ID into a provided character buffer.
    // Format: YYYY-MM-DD-HHhMMminSSsec-{salt}
    //
    static String get_next_GUID(const String& salt)
    {
        VERIFY( 0 < salt.size && salt.size <= 64, "salt.size must be in ]1, 32]");

        // Get current time
        auto now = std::chrono::system_clock::now();
        std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);

        // Convert to local time
        std::tm time_buf;
        std::tm* local_tm = nullptr;

    #if defined(_WIN32) || defined(_MSC_VER)
        ASSERT(localtime_s(&time_buf, &now_time_t) == 0);
        local_tm = &time_buf;
    #else
        local_tm = localtime_r(&now_time_t, &time_buf);
        ASSERT(local_tm);
    #endif

        String result = string_printf(
            temp_allocator(),
            "%04d-%02d-%02d-%02dh%02dm%02ds-%s-0x%016llx",
            local_tm->tm_year + 1900,
            local_tm->tm_mon + 1,
            local_tm->tm_mday,
            local_tm->tm_hour,
            local_tm->tm_min,
            local_tm->tm_sec,
            salt.c_str(),
            g_unique_id_generator_counter++
        );

        return result;
    }

}