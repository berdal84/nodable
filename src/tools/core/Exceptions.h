#pragma once

#include <cpptrace/cpptrace.hpp>

namespace tools
{
    typedef cpptrace::runtime_error runtime_error;
    typedef cpptrace::domain_error  domain_error;

    class unreachable : public domain_error
    {
    public:
        unreachable(): domain_error("This code should not be reachable") {}
    };
}