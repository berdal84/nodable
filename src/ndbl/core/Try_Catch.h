#pragma once

#include <cstdlib>
#include <iostream>
#include <memory>
#include <cstddef>

#ifndef NDBL_TRY_CATCH_ENABLE
#   define NDBL_TRY_CATCH_ENABLE true
#endif

#if NDBL_TRY_CATCH_ENABLE

#define NDBL_try \
    try

#define NDBL_catch \
    catch(const std::exception & std_error) \
    { \
        std::cout << std_error.what() << std::flush; \
        exit(1); \
    }

#endif // #if NDBL_TRY_CATCH_ENABLE