#include <benchmark/benchmark.h>
#include "Pool.h"

using namespace fw;

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
    std::vector< DataPool<127>* > ptrs2;
    ptrs.reserve( COUNT );
    for( auto i = 0; i < COUNT; i++ )
    {
        ptrs.emplace_back( new DataPool<128>() );
        ptrs2.emplace_back( new DataPool<127>() );
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

template<size_t COUNT, bool USE_VECTOR>
static void mutate_N_instances__enterlaced_with_another_type__using_Pool_create(benchmark::State& state)
{
    Pool::init();
    Pool* pool = Pool::get_pool();
    pool->init_for<DataPool<128>>();
    pool->init_for<DataPool<127>>();

    std::vector<PoolID<DataPool<128>>> ids;
    std::vector<PoolID<DataPool<127>>> ids2;
    ids.reserve(COUNT);
    for( auto i = 0; i < COUNT; i++ )
    {
        ids.emplace_back( pool->create<DataPool<128>>() );
        ids2.emplace_back( pool->create<DataPool<127>>() );
    }

    if( USE_VECTOR )
    {
        auto& vector = pool->get_all<DataPool<128>>();
        for ( auto _ : state )
        {
            // benchmark begin
            for( auto& each : vector )
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
        for ( auto _ : state )
        {
            // benchmark begin

            // first we dereference all the ids
            std::array<DataPool<128>*, COUNT> ptrs;
            for( size_t i = 0; i < COUNT; i++ )
            {
                ptrs[i] = ids[i].get();
            }

            // then we iterate (and mutate)
            for( DataPool<128>* each : ptrs )
            {
                each->data[0] = 'X';
                each->data[63] = 'Y';
                each->data[127] = 'Z';
            }
            // benchmark end
        }
    }
    Pool::shutdown();
}

#define ENTERLACED( N ) \
BENCHMARK( mutate_N_instances__enterlaced_with_another_type__using_new<N> ); \
BENCHMARK( mutate_N_instances__enterlaced_with_another_type__using_Pool_create<N, false> ); \
BENCHMARK( mutate_N_instances__enterlaced_with_another_type__using_Pool_create<N, true> );

ENTERLACED(2)
ENTERLACED(10)
ENTERLACED(50)
ENTERLACED(100)
ENTERLACED(1000)
ENTERLACED(10000)
ENTERLACED(100000)

BENCHMARK_MAIN();
