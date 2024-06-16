#ifdef TOOLS_POOL_ENABLE
#include "tools/core/memory/memory.h"
#include "tools/core/log.h"
#include <gtest/gtest.h>

using namespace tools;

class Data {
public:
    Data(): name("") {}
    explicit Data(const char* _name): name( _name ){}
    const char* name;
    POOL_REGISTRABLE(Data)
};

TEST(Pool, init_shutdown )
{
    EXPECT_ANY_THROW( shutdown_pool_manager() ); // not initialized !
    init_pool_manager();
    EXPECT_ANY_THROW( init_pool_manager() ); // twice
    shutdown_pool_manager();
    EXPECT_ANY_THROW( shutdown_pool_manager() ); // twice
}

TEST(Pool, create_empty_constructor )
{
    Pool* pool = init_pool_manager()->get_pool();
    pool->init_for<Data>();
    Data> node = pool->create<Data>();
    EXPECT_NE(node, Data>::null);
    EXPECT_NE(node.get(), nullptr);
    shutdown_pool_manager();
}

TEST(Pool, create_with_args )
{
    PoolManager* pool_manager = init_pool_manager(); Pool* pool = pool_manager->get_pool();
    pool->init_for<Data>();
    Data> node = pool->create<Data>("Toto");
    EXPECT_NE(node, Data>::null);
    EXPECT_NE(node.get(), nullptr);
    EXPECT_STREQ(node->name, "Toto");
    shutdown_pool_manager();
}

TEST(Pool, buffer_resizing )
{
    PoolManager* pool_manager = init_pool_manager(); Pool* pool = pool_manager->get_pool();
    pool->init_for<Data>();
    Data> node1 = pool->create<Data>("Toto");
    Data> node2 = pool->create<Data>("Tata");
    EXPECT_EQ((u64_t)node1, 0);
    EXPECT_EQ((u64_t)node2, 1);
    EXPECT_STREQ(node1->name, "Toto");
    EXPECT_STREQ(node2->name, "Tata");
    shutdown_pool_manager();
}

TEST(Pool, destroy_last )
{
    PoolManager* pool_manager = init_pool_manager(); Pool* pool = pool_manager->get_pool();
    pool->init_for<Data>();
    Data> data_1 = pool->create<Data>("Toto");
    Data> data_2 = pool->create<Data>("Tata");
    EXPECT_EQ(pool->get_all<Data>().size(), 2);
    pool->destroy( data_1 );
    EXPECT_STREQ(data_2->name, "Tata");
    EXPECT_EQ(pool->get_all<Data>().size(), 1);
    shutdown_pool_manager();
}


TEST(Pool, destroy_first )
{
    PoolManager* pool_manager = init_pool_manager(); Pool* pool = pool_manager->get_pool();
    pool->init_for<Data>();
    Data> node1 = pool->create<Data>("Toto");
    Data> node2 = pool->create<Data>("Tata");
    EXPECT_EQ(pool->get_all<Data>().size(), 2);
    pool->destroy( node1 );
    EXPECT_EQ(pool->get_all<Data>().size(), 1);
    EXPECT_STREQ(node2->name, "Tata");
    shutdown_pool_manager();
}

TEST(Pool, destroy_first_and_reuse_id )
{
    // prepare
    Pool* pool = init_pool_manager()->get_pool();
    pool->init_for<Data>();
    Data> node1 = pool->create<Data>("Toto");
    Data> node2 = pool->create<Data>("Tata");

    // act
    pool->destroy( node1 );
    Data> node3 = pool->create<Data>("Tutu");
    Data> node4 = pool->create<Data>("Tete");

    // clean
    EXPECT_EQ((u64_t)node3, 0);
    EXPECT_EQ((u64_t)node4, 2 );
    shutdown_pool_manager();
}

TEST(Pool, destroy_vector_of_ids )
{
    size_t n = 200;
    Pool* pool = init_pool_manager()->get_pool();
    pool->init_for<Data>();

    std::vector<Data>> data;
    for(int i = 0; i < n; ++i)
    {
        data.push_back( pool->create<Data>("Data") );
    }

    auto& all_data = pool->get_all<Data>();
    EXPECT_EQ(all_data.size(), n);

    pool->destroy_all( data );

    EXPECT_EQ(all_data.size(), 0 );

    shutdown_pool_manager();
}

TEST(Pool, reserve_size )
{
    constexpr int N = 128;

    PoolManager::Config cfg;
    cfg.default_pool_config.reserved_size = N;
    Pool* pool = init_pool_manager(cfg)->get_pool();
    pool->init_for<Data>();

    // Create n Data and store their address just after creation
    std::vector<Data*> pointers;
    pointers.reserve( N );
    for(int i = 0; i < N; ++i)
    {
        pointers.push_back( pool->create<Data>("Data").get() );
    }

    // check if addresses are identical
    std::vector<Data>& data = pool->get_all<Data>();
    for(int i = 0; i < N; ++i)
    {
        EXPECT_EQ( &data[i], pointers[i] );
    }

    // Push a 129th Data (should resize and ann addresses should change except for the last one)
    pointers.push_back( pool->create<Data>("Data").get() );
    for(int i = 0; i < N; ++i)
    {
        EXPECT_NE( &data[i], pointers[i] );
    }
    EXPECT_EQ( &data[N], pointers[N] );

    shutdown_pool_manager();
}

template<size_t S>
struct TData {
    POOL_REGISTRABLE(TData)
    char data[S];
};

TEST(PoolVector, create_delete )
{
    auto* ptr1 = new TPoolVector<TData<128>>();
    auto* ptr2 = new TPoolVector<TData<127>>();
    delete ptr1;
    delete ptr2;
}

#endif // #ifdef TOOLS_POOL_ENABLE
