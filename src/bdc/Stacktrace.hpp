#pragma once

#include <ostream>    // for std::cout, std::endl
#include <stacktrace> // for std::stacktrace
#define BDC_PRINT_STACKTRACE() \
    std::stacktrace st = std::stacktrace::current(); \
    std::cout << st << std::endl