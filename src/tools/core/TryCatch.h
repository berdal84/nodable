#pragma once

#include <cstdlib>
#include <memory>
#include <cstddef>
#include <stdexcept>

#ifndef TOOLS_TRY_CATCH_ENABLE
#   define TOOLS_TRY_CATCH_ENABLE true
#endif

#if TOOLS_TRY_CATCH_ENABLE

#define TOOLS_try \
    try

#define TOOLS_catch \
    catch(const std::logic_error& e) \
    { \
        std::cout << e.what() << std::flush; \
        exit(1); \
    } \
    catch(const std::exception & e) \
    { \
        std::cout << e.what() << std::flush; \
        exit(1); \
    }

#endif // #if TOOLS_TRY_CATCH_ENABLE