#pragma once

#include "MACROS.hpp" // for BDC_LOG, etc.

static const char*  g_suite_name = "invalid";
static size_t       g_suite_test_failures;
static size_t       g_suite_test_count;
static size_t       g_suite_warnings;

static const char*  g_test_name = "invalid";
static size_t       g_test_expect_failures;
static size_t       g_test_expect_count;

#define TEST_SUITE_BEGIN( name ) \
g_suite_warnings      = 0; \
g_suite_test_failures = 0; \
g_suite_test_count    = 0; \
g_suite_name          = #name; \
BDC_LOG("\n{} - Running Test Suite ...\n", g_suite_name); \
try

#define TEST_SUITE_END \
catch (std::runtime_error error) \
{ \
    g_test_expect_failures++; \
} \
if( g_suite_warnings ) \
{ \
    BDC_LOG("{} WARNING: no tests are present, use TEST_BEGIN/TEST_END macros.\n", g_suite_name); \
} \
else if ( g_suite_test_failures ) \
{ \
    BDC_LOG("{} FAILED!\n", g_suite_name); \
    BDC_LOG("-- {}/{} test(s) failed, see error(s) above in the console.\n", g_suite_test_failures, g_suite_test_count ); \
    BDC_LOG("Abording program..."); \
    exit(1); \
} \
else \
{ \
    BDC_LOG("{} PASSED\n", g_suite_name); \
}

#define TEST_BEGIN( name ) \
g_suite_test_count++; \
g_test_expect_failures = 0; \
g_test_expect_count    = 0; \
g_test_name = #name; \
BDC_LOG("    {} ...\n", g_test_name); \
try

#define TEST_END \
catch (std::runtime_error error) \
{ \
    g_test_expect_failures++; \
} \
if( g_test_expect_count ) \
{ \
    if ( g_test_expect_failures ) \
    { \
        BDC_LOG("    {} FAILED! ({}/{} expectation(s) failed)\n", g_test_name, g_test_expect_failures, g_test_expect_count+g_test_expect_failures); \
        g_suite_test_failures++; \
    } \
    else \
    { \
        BDC_LOG("    {} PASSED\n", g_test_name); \
    } \
} \
else \
{ \
    BDC_LOG("    {} WARNING: Nothing to test here, use TEST_EXPECTS macro.\n", g_test_name); \
    g_suite_warnings++; \
}

#define TEST_EXPECTS( ... ) \
do \
{ \
    g_test_expect_count++; \
    if ( !(__VA_ARGS__) ) \
    { \
        g_test_expect_failures++; \
        BDC_LOG( \
            "\n\n" \
            "-- [KO] TEST_EXPECTS({}) was evaluated false.\n" \
            "        See code in file {} line {}\n\n", #__VA_ARGS__, __FILE__, __LINE__); \
    } \
} while(0)
