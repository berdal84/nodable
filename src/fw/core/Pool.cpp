#include "Pool.h"

using namespace fw;

Pool* Pool::s_current_pool = nullptr;

Pool* Pool::init(size_t reserved_size)
{
    if( s_current_pool != nullptr )
    {
        LOG_WARNING("Pool", "Pool is already initialized");
        return s_current_pool;
    }

    s_current_pool = new Pool(reserved_size);
    return s_current_pool;
}

void Pool::shutdown()
{
    FW_EXPECT(s_current_pool != nullptr, "No current pool. Did you called shutdown more than once?")
    delete s_current_pool;
    s_current_pool = nullptr;
}

Pool::Pool(size_t reserved_size)
    : m_reserved_size( reserved_size )
    , m_next_id( 1 ) // Reserve 0 for null
{
}

Pool::~Pool()
{
    for(auto& [_, each_vec] : m_vector_by_type )
    {
        delete each_vec;
    }
}