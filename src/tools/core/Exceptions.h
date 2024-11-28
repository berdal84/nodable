#pragma once
#include <stdexcept>

namespace tools
{
    class unreachable : public std::logic_error
    {
    public:
        unreachable(): std::logic_error("This code should not be reachable") {}
    };
}

