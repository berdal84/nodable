#include "Pool.h"
#include "core/log.h"
#include <gtest/gtest.h>

using namespace fw;

class Data {
public:
    Data(): name("") {}
    explicit Data(const char* _name): name( _name ){}
    const char* name;
    POOL_REGISTRABLE(Data)
};

TEST(Pool, init_shutdown )
{
    EXPECT_ANY_THROW( Pool::shutdown() ); // not initialized !
    Pool::init();
    EXPECT_ANY_THROW( Pool::init() ); // twice
    Pool::shutdown();
    EXPECT_ANY_THROW( Pool::shutdown() ); // twice
}

TEST(Pool, create_empty_constructor )
{
    Pool* pool = Pool::init(0);
    pool->init_for<Data>();
    PoolID<Data> node = pool->create<Data>();
    EXPECT_NE(node, PoolID<Data>::null);
    EXPECT_NE(node.get(), nullptr);
    Pool::shutdown();
}

TEST(Pool, create_with_args )
{
    Pool* pool = Pool::init(0);
    pool->init_for<Data>();
    PoolID<Data> node = pool->create<Data>("Toto");
    EXPECT_NE(node, PoolID<Data>::null);
    EXPECT_NE(node.get(), nullptr);
    EXPECT_STREQ(node->name, "Toto");
    Pool::shutdown();
}

TEST(Pool, buffer_resizing )
{
    Pool* pool = Pool::init(0);
    pool->init_for<Data>();
    PoolID<Data> node1 = pool->create<Data>("Toto");
    PoolID<Data> node2 = pool->create<Data>("Tata");
    EXPECT_EQ((u64_t)node1, 0);
    EXPECT_EQ((u64_t)node2, 1);
    EXPECT_STREQ(node1->name, "Toto");
    EXPECT_STREQ(node2->name, "Tata");
    Pool::shutdown();
}

TEST(Pool, destroy_last )
{
    Pool* pool = Pool::init(0);
    pool->init_for<Data>();
    PoolID<Data> data_1 = pool->create<Data>("Toto");
    PoolID<Data> data_2 = pool->create<Data>("Tata");
    EXPECT_EQ(pool->get_all<Data>().size(), 2);
    pool->destroy( data_1 );
    EXPECT_STREQ(data_2->name, "Tata");
    EXPECT_EQ(pool->get_all<Data>().size(), 1);
    Pool::shutdown();
}


TEST(Pool, destroy_first )
{
    Pool* pool = Pool::init(0);
    pool->init_for<Data>();
    PoolID<Data> node1 = pool->create<Data>("Toto");
    PoolID<Data> node2 = pool->create<Data>("Tata");
    EXPECT_EQ(pool->get_all<Data>().size(), 2);
    pool->destroy( node1 );
    EXPECT_EQ(pool->get_all<Data>().size(), 1);
    EXPECT_STREQ(node2->name, "Tata");
    Pool::shutdown();
}

TEST(Pool, destroy_first_and_reuse_id )
{
    // prepare
    Pool* pool = Pool::init(0);
    pool->init_for<Data>();
    PoolID<Data> node1 = pool->create<Data>("Toto");
    PoolID<Data> node2 = pool->create<Data>("Tata");

    // act
    pool->destroy( node1 );
    PoolID<Data> node3 = pool->create<Data>("Tutu");
    PoolID<Data> node4 = pool->create<Data>("Tete");

    // clean
    EXPECT_EQ((u64_t)node3, 0);
    EXPECT_EQ((u64_t)node4, 2 );
    Pool::shutdown();
}

TEST(Pool, destroy_vector_of_ids )
{
    size_t n = 200;
    Pool* pool = Pool::init();
    pool->init_for<Data>();

    std::vector<PoolID<Data>> data;
    for(int i = 0; i < n; ++i)
    {
        data.push_back( pool->create<Data>("Data") );
    }

    auto& all_data = pool->get_all<Data>();
    EXPECT_EQ(all_data.size(), n);

    pool->destroy_all( data );

    EXPECT_EQ(all_data.size(), 0 );

    Pool::shutdown();
}

TEST(Pool, reserve_size )
{
    int n = 128;
    Pool* pool = Pool::init( n );
    pool->init_for<Data>();

    // Create n Data and store their address just after creation
    std::vector<Data*> pointers;
    pointers.reserve(512);
    for(int i = 0; i < n; ++i)
    {
        pointers.push_back( pool->create<Data>("Data").get() );
    }

    // check if addresses are identical
    std::vector<Data>& data = pool->get_all<Data>();
    for(int i = 0; i < n; ++i)
    {
        EXPECT_EQ( &data[i], pointers[i] );
    }

    // Push a 129th Data (should resize and ann addresses should change except for the last one)
    pointers.push_back( pool->create<Data>("Data").get() );
    for(int i = 0; i < n; ++i)
    {
        EXPECT_NE( &data[i], pointers[i] );
    }
    EXPECT_EQ( &data[n], pointers[n] );

    Pool::shutdown();
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

