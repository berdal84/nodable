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

#define INSTANCE_COUNT 200

static void iterate_n_instances__NOT_using_pool(benchmark::State& state)
{
    for ( auto _ : state )
    {
        state.PauseTiming();
        std::vector< DataPool<128>* > ptrs;
        ptrs.reserve(INSTANCE_COUNT);
        for( auto i = 0; i < INSTANCE_COUNT; i++ )
        {
            ptrs.emplace_back( new DataPool<128>() );
        }
        state.ResumeTiming();

        for( DataPool<128>* each : ptrs )
        {
            each->data[0] = '1';
        }

        state.PauseTiming();
        for( auto& each : ptrs ) delete each;
        ptrs.clear();
        state.ResumeTiming();
    }
}

static void iterate_n_instances__using_pool(benchmark::State& state)
{
    Pool::init();
    Pool::get_pool()->init_for<DataPool<128>>();

    for ( auto _ : state )
    {
        state.PauseTiming();
        std::vector<PoolID<DataPool<128>>> ptrs;
        ptrs.reserve(INSTANCE_COUNT);
        for( auto i = 0; i < INSTANCE_COUNT; i++ )
        {
            ptrs.emplace_back( Pool::get_pool()->create<DataPool<128>>() );
        }
        state.ResumeTiming();

        for( DataPool<128>& each : Pool::get_pool()->get_all<DataPool<128>>() )
        {
            each.data[0] = '1';
        }

        state.PauseTiming();
        for(auto& each : ptrs ) Pool::get_pool()->destroy(each);
        ptrs.clear();
        state.ResumeTiming();
    }
    Pool::shutdown();
}

static void create_destroy__using_pool(benchmark::State& state)
{
    std::vector<PoolID<DataPool<128>>> ptrs;
    ptrs.reserve(INSTANCE_COUNT);
    Pool::init();
    Pool::get_pool()->init_for<DataPool<128>>();
    for ( auto _ : state )
    {
        for( auto i = 0; i < INSTANCE_COUNT; i++ )
        {
            ptrs.emplace_back( Pool::get_pool()->create<DataPool<128>>() );
        }
        for( auto ptr : ptrs )
        {
            Pool::get_pool()->destroy(ptr);
        }
        state.PauseTiming();
        ptrs.clear();
        state.ResumeTiming();
    }
    Pool::shutdown();
}

static void create_destroy__NOT_using_pool(benchmark::State& state)
{
    std::vector<Data<128>*> ptrs;
    ptrs.reserve( INSTANCE_COUNT );
    for ( auto _ : state )
    {
        for( auto i = 0; i < INSTANCE_COUNT; i++ )
        {
            ptrs.push_back( new Data<128>() );
        }
        for( auto ptr : ptrs )
        {
            delete ptr;
        }
        state.PauseTiming();
        ptrs.clear();
        state.ResumeTiming();
    }
}

BENCHMARK( iterate_n_instances__NOT_using_pool );
BENCHMARK( iterate_n_instances__using_pool );
BENCHMARK( create_destroy__using_pool );
BENCHMARK( create_destroy__NOT_using_pool );

BENCHMARK_MAIN();
