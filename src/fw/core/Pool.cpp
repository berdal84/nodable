#include "Pool.h"

using namespace fw;

Pool* Pool::s_current_pool = nullptr;

Pool* Pool::init(size_t _capacity, bool _reuse_ids)
{
    FW_EXPECT( s_current_pool == nullptr, "Should initialize Pool once" )
    s_current_pool = new Pool( _capacity, _reuse_ids);
    return s_current_pool;
}

void Pool::shutdown()
{
    FW_EXPECT(s_current_pool != nullptr, "No current pool. Did you called shutdown more than once?")
    delete s_current_pool;
    s_current_pool = nullptr;
}

Pool::Pool(size_t _capacity, bool _reuse_ids)
    : m_initial_capacity( _capacity )
    , m_reuse_ids(_reuse_ids)
    , m_first_free_id( invalid_id<u32_t> )
    , m_pool_vector_by_type()
    , m_record_by_id()
{
}

Pool::~Pool()
{
    for(const auto& [type, pool_vector]: m_pool_vector_by_type )
    {
        delete pool_vector;
    }
}