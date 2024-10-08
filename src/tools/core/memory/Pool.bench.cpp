#include <benchmark/benchmark.h>
#include "tools/core/memory/memory.h"

using namespace tools;

template<size_t SIZE>
class Data {
public:
    Data()
    { memset( data, 'X', SIZE); }
    ~Data(){}
    char data[SIZE];
};


template<size_t SIZE>
class DataPool {
public:
    POOL_REGISTRABLE(DataPool<SIZE>);
    DataPool()
    { memset( data, '0', SIZE); }
    ~DataPool(){}
    char data[SIZE];
};

template<size_t COUNT>
static void mutate_N_instances__enterlaced_with_another_type__using_new(benchmark::State& state)
{
    std::vector< DataPool<128>* > ptrs;
    std::vector< DataPool<64>* > ptrs2;
    ptrs.reserve( COUNT );
    for( auto i = 0; i < COUNT; i++ )
    {
        ptrs.emplace_back( new DataPool<128>() );
        ptrs2.emplace_back( new DataPool<64>() );
    }
    for ( auto _ : state )
    {
        // benchmark begin
        for( auto* each : ptrs )
        {
            each->data[0] = 'X';
            each->data[63] = 'Y';
            each->data[127] = 'Z';
        }
        // benchmark end
    }
    for( auto& each : ptrs ) delete each;
    for( auto& each : ptrs2 ) delete each;
}


constexpr bool USE_VECTOR = true;

template<size_t COUNT, bool VECTOR>
static void mutate_N_instances__enterlaced_with_another_type__using_Pool_create(benchmark::State& state)
{
    PoolManager* pool_manager = init_pool_manager();
    Pool*        pool         = pool_manager->get_pool();

    pool->init_for<DataPool<128>>();
    pool->init_for<DataPool<64>>();

    std::vector<DataPool<128>>> ids;
    std::vector<DataPool<64>>> ids2;

    for( auto i = 0; i < COUNT; i++ )
    {
        ids.emplace_back( pool->create<DataPool<128>>() );
        ids2.emplace_back( pool->create<DataPool<64>>() );
    }

    if( VECTOR )
    {
        for ( auto _ : state )
        {
            // benchmark begin
            for( auto& each : pool->get_all<DataPool<128>>() )
            {
                each.data[0] = 'X';
                each.data[63] = 'Y';
                each.data[127] = 'Z';
            }
            // benchmark end
        }
    }
    else
    {
        std::vector<DataPool<128>*> pointers;
        for ( auto _ : state )
        {
            // benchmark begin
            pointers.resize(ids.size()); // note: we know that this will cost a dynamic alloc once per benchmark and not each call.
            pool->get(pointers, ids);
            for( DataPool<128>* each : pointers )
            {
                each->data[0] = 'X';
                each->data[63] = 'Y';
                each->data[127] = 'Z';
            }
            // benchmark end
        }
    }
    shutdown_pool_manager();
}

#define ENTERLACED( N ) \
BENCHMARK( mutate_N_instances__enterlaced_with_another_type__using_new<N> ); \
BENCHMARK( mutate_N_instances__enterlaced_with_another_type__using_Pool_create<N, USE_VECTOR> ); \
BENCHMARK( mutate_N_instances__enterlaced_with_another_type__using_Pool_create<N, !USE_VECTOR> );

ENTERLACED(2)
ENTERLACED(10)
ENTERLACED(50)
ENTERLACED(100)
ENTERLACED(1000)
ENTERLACED(10000)
ENTERLACED(100000)

BENCHMARK_MAIN();
