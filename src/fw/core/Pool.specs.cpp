#include <gtest/gtest.h>
#include "log.h"
#include "Pool.h"

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
    EXPECT_ANY_THROW( Pool::init() ); // twice!
    Pool::shutdown();
    EXPECT_ANY_THROW( Pool::shutdown() ); // twice
}

TEST(Pool, create_empty_constructor )
{
    Pool* pool = Pool::init(0);
    PoolID<Data> node = pool->create<Data>();
    EXPECT_NE(node, PoolID<Data>::null);
    EXPECT_NE(node.get(), nullptr);
    Pool::shutdown();
}

TEST(Pool, create_with_args )
{
    Pool* pool = Pool::init(0);
    PoolID<Data> node = pool->create<Data>("Toto");
    EXPECT_NE(node, PoolID<Data>::null);
    EXPECT_NE(node.get(), nullptr);
    EXPECT_STREQ(node->name, "Toto");
    Pool::shutdown();
}

TEST(Pool, buffer_resizing )
{
    Pool* pool = Pool::init(0);
    PoolID<Data> node1 = pool->create<Data>("Toto");
    PoolID<Data> node2 = pool->create<Data>("Tata");
    EXPECT_EQ(node1, size_t(1));
    EXPECT_EQ(node2, size_t(2));
    EXPECT_STREQ(node1->name, "Toto");
    EXPECT_STREQ(node2->name, "Tata");
    Pool::shutdown();
}

TEST(Pool, destroy_last )
{
    Pool* pool = Pool::init(0);
    PoolID<Data> data_1 = pool->create<Data>("Toto");
    PoolID<Data> data_2 = pool->create<Data>("Tata");
    EXPECT_EQ(pool->get_all<Data>().size(), 2);
    pool->destroy( data_1 );
    EXPECT_EQ(data_2->name, "Tata");
    EXPECT_EQ(pool->get_all<Data>().size(), 1);
    Pool::shutdown();
}


TEST(Pool, destroy_first )
{
    Pool* pool = Pool::init(0);
    PoolID<Data> node1 = pool->create<Data>("Toto");
    PoolID<Data> node2 = pool->create<Data>("Tata");
    EXPECT_EQ(pool->get_all<Data>().size(), 2);
    pool->destroy( node1 );
    EXPECT_EQ(pool->get_all<Data>().size(), 1);
    EXPECT_EQ(node2->name, "Tata");
    Pool::shutdown();
}

TEST(Pool, destroy_vector_of_ids )
{
    size_t n = 200;
    Pool* pool = Pool::init();

    std::vector<PoolID<Data>> data;
    for(int i = 0; i < n; ++i)
    {
        data.push_back( pool->create<Data>("Data") );
    }

    EXPECT_EQ(pool->get_all<Data>().size(), n);
    EXPECT_EQ(pool->get_all<Data>().capacity(), 256 );

    pool->destroy( data );

    EXPECT_EQ(pool->get_all<Data>().size(), 0 );


    Pool::shutdown();
}

TEST(Pool, reserve_size )
{
    int n = 128;
    Pool* pool = Pool::init( n );

    // Create n Data and store their address just after creation
    std::vector<Data*> pointers;
    pointers.reserve(n);
    for(int i = 0; i < n; ++i)
    {
        pointers.push_back( pool->create<Data>("Data").get() );
    }

    // check if addresses are identical
    auto& data = pool->get_all<Data>();
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

