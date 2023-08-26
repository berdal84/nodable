#include "Pool.h"

using namespace fw;
using namespace fw::pool;

Pool* Pool::s_current_pool = nullptr;

void* Record::data() const
{
    return vector->at(pos);
}

Pool* Pool::get_pool()
{
    FW_EXPECT(s_current_pool != nullptr, "No pool. Did you called Pool::init() ?")
    return s_current_pool;
}

Pool* Pool::init(size_t reserved_size)
{
    FW_EXPECT(s_current_pool == nullptr, "Already initialized");
    s_current_pool = new Pool(reserved_size);
    return s_current_pool;
}

void Pool::shutdown()
{
    FW_EXPECT(s_current_pool != nullptr, "No current pool. Did you called shutdown more than once?")
    delete s_current_pool;
    s_current_pool = nullptr;
}

i32_t Pool::generate_id()
{
    return m_next_id++;
}

Pool::Pool(size_t reserved_size)
    : m_reserved_size( reserved_size )
    , m_next_id( ID_NULL + 1 )
{}

Pool::~Pool()
{
    for(auto& [_, each_vec] : m_vector_by_type )
    {
        delete each_vec;
    }
}

AgnosticVector::~AgnosticVector()
{
    delete_buffer();
}
