#include <benchmark/benchmark.h>
#include <random>
#include "string.h"

using namespace fw;

template<class StringT>
static void BM_empty_constructor(benchmark::State& state)
{
    for (auto _ : state)
    {
        StringT str;
        benchmark::DoNotOptimize(str);
    }
}

template<class StringT>
static void BM_constructor(benchmark::State& state)
{
    auto size = state.range(0);
    char* buf = new char[size];
    memset(buf, 'x', size);
    buf[size-1] = 0;
    for (auto _ : state)
    {
        StringT str{buf};
    }
    delete[] buf;
}

template<class StringT>
static void BM_constructor_then_push_back_a_char(benchmark::State& state)
{
    char* buf = new char[state.range(0)];
    memset(buf, 'x', state.range(0));
    buf[state.range(0)-1] = 0;
    for (auto _ : state)
    {
        StringT str{buf};
        str.push_back('+');
    }
    delete[] buf;
}

template<size_t STRING_SIZE>
static void BM_strncpy_stack_to_stack(benchmark::State& state)
{
    char source[STRING_SIZE];
    while (state.KeepRunning())
    {
        char destination[STRING_SIZE];
        strncpy(destination, source, STRING_SIZE);
        benchmark::DoNotOptimize(destination);
    }
}

template<size_t STRING_SIZE>
static void BM_strncpy_heap_to_stack(benchmark::State& state)
{
    char* source = new char[STRING_SIZE];
    while (state.KeepRunning())
    {
        char destination[STRING_SIZE];
        strncpy(destination, source, STRING_SIZE);
        benchmark::DoNotOptimize(destination);
    }
    delete[] source;
}

template<size_t STRING_SIZE>
static void BM_strncpy_heap_to_heap(benchmark::State& state)
{
    char* source = new char[STRING_SIZE];
    char* destination = new char[STRING_SIZE];
    while (state.KeepRunning())
    {
        strncpy(destination, source, STRING_SIZE);
        benchmark::DoNotOptimize(destination);
    }
    delete[] source;
    delete[] destination;
}

BENCHMARK(BM_empty_constructor<std::string>);
BENCHMARK(BM_empty_constructor<string>);
BENCHMARK(BM_empty_constructor<string64>);
BENCHMARK(BM_empty_constructor<string128>);

BENCHMARK(BM_constructor<std::string>)->Range(1, 16);
BENCHMARK(BM_constructor<string>)->Range(1, 16);
BENCHMARK(BM_constructor<string8>)->Range(1, 16);
BENCHMARK(BM_constructor<string16>)->Range(1, 16);
BENCHMARK(BM_constructor<string32>)->Range(1, 16);

BENCHMARK(BM_constructor_then_push_back_a_char<std::string>)->Range(1, 16);
BENCHMARK(BM_constructor_then_push_back_a_char<string>)->Range(1, 16);
BENCHMARK(BM_constructor_then_push_back_a_char<string8>)->Range(1, 16);
BENCHMARK(BM_constructor_then_push_back_a_char<string16>)->Range(1, 16);
BENCHMARK(BM_constructor_then_push_back_a_char<string32>)->Range(1, 16);


#define BENCHMARK_STRNCPY(BM_strncpy_, size) \
BENCHMARK(BM_strncpy_<size/8>); \
BENCHMARK(BM_strncpy_<size/4>); \
BENCHMARK(BM_strncpy_<size/2>); \
BENCHMARK(BM_strncpy_<size>); \
BENCHMARK(BM_strncpy_<size*2>); \
BENCHMARK(BM_strncpy_<size*4>); \
BENCHMARK(BM_strncpy_<size*8>); \
BENCHMARK(BM_strncpy_<size*16>); \
BENCHMARK(BM_strncpy_<size*32>);

constexpr size_t _32KiB = 32 * 1024;
BENCHMARK_STRNCPY(BM_strncpy_stack_to_stack, _32KiB)
BENCHMARK_STRNCPY(BM_strncpy_heap_to_stack, _32KiB)
BENCHMARK_STRNCPY(BM_strncpy_heap_to_heap, _32KiB)

BENCHMARK_MAIN();