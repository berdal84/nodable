#ifdef TOOLS_DEBUG
#include "MemoryManager.h"
#include "tools/core/log.h"
#include <gtest/gtest.h>

using namespace tools;

TEST(MemoryManager, init_shutdown )
{
    init_memory_manager();
    shutdown_memory_manager();
}

TEST(MemoryManager, get_memory_stats )
{
    init_memory_manager();
    const MemoryStats* stats = get_memory_stats();
    EXPECT_NE(stats, nullptr);
    shutdown_memory_manager();
}

void new_delete(size_t size_to_allocate);

TEST(MemoryManager, new_delete_0char )
{
    new_delete( 0 );// new char[0] should return nullptr
}

TEST(MemoryManager, new_delete_1char_and_more )
{
    new_delete(4096);
}

void new_delete(const size_t size_to_allocate)
{
    init_memory_manager();
    const MemoryStats* stats = get_memory_stats();

    size_t initial_memory_usage = stats->mem_usage();
    size_t initial_alloc_count = stats->alloc_count();

    char* ptr = new char[size_to_allocate];

    size_t after_new_memory_usage = stats->mem_usage();
    size_t after_new_alloc_count = stats->alloc_count();
    size_t expected_alloc_count = size_to_allocate == 0 ? 0 : 1; // no alloc is expected for new char[0]
    EXPECT_EQ( after_new_alloc_count, initial_alloc_count + expected_alloc_count);
    EXPECT_EQ( after_new_memory_usage, initial_memory_usage + size_to_allocate );

    delete[] ptr;

    size_t after_delete_memory_usage = stats->mem_usage();
    size_t after_delete_allocation_count = stats->alloc_count();
    EXPECT_EQ( after_delete_allocation_count, initial_alloc_count );
    EXPECT_EQ( after_delete_memory_usage, initial_alloc_count );

    shutdown_memory_manager();
}
#endif
