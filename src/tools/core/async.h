#pragma once
#include "types.h"
#include <functional>

namespace tools
{
    /**
     * Handle asynchronous tasks
     * @example @code
     * async::delay([]()-> void {...}), 60)
     */
    namespace async
    {
        void clean_tasks(); // remove finished tasks
        void delay(const std::function<void(void)>& function, u64_t delay_in_ms );
    };
}