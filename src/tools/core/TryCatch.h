#pragma once

#include <cstdlib>
#include <memory>
#include <cstddef>
#include <cpptrace/cpptrace.hpp>

#ifndef TOOLS_TRY_CATCH_ENABLE
#   define TOOLS_TRY_CATCH_ENABLE true
#endif

#if TOOLS_TRY_CATCH_ENABLE

#define TOOLS_try \
    try

#define TOOLS_catch \
    catch(const cpptrace::logic_error& logic_error) \
    { \
        logic_error.trace().print_with_snippets();  \
        std::cout << std::flush; \
        exit(1); \
    } \
    catch(const std::exception & std_error) \
    { \
        std::cout << std_error.what() << std::flush; \
        exit(1); \
    }

#endif // #if TOOLS_TRY_CATCH_ENABLE