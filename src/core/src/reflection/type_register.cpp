#include <nodable/core/reflection/type_register.h>
#include <nodable/core/reflection/type.h>
#include <nodable/core/Log.h>
#include <nodable/core/assertions.h>

using namespace ndbl;

type type_register::get(size_t _hash)
{
    auto found = by_hash().find(_hash);
    NDBL_ASSERT_EX(found != by_hash().end(), "reflection: type not found!")
    return found->second;
}

std::map<size_t, type>& type_register::by_hash()
{
    static std::map<size_t, type> meta_type_register_by_typeid;
    return meta_type_register_by_typeid;
}


bool type_register::has(type _type)
{
    return by_hash().find(_type.hash_code()) != by_hash().end();
}

bool type_register::has(size_t _hash_code)
{
    auto found = by_hash().find(_hash_code);
    return found != by_hash().end();
}

void type_register::insert(type _type)
{
    // insert if absent from register
    if( !has(_type.hash_code()))
    {
        by_hash().insert({_type.hash_code(), _type});
        return;
    }

    // merge with existing
    type existing = get(_type.hash_code());
    LOG_VERBOSE("reflection", "Merge existing: \"%s\" (%s), with: \"%s\" (%s)\n"
    , existing.m_name.c_str(), existing.m_compiler_name.c_str()
    , _type.m_name.c_str(), _type.m_compiler_name.c_str()
    )
    if( _type.m_name.empty() ) _type.m_name = existing.m_name;
    _type.m_children.insert(existing.m_children.begin(), existing.m_children.end() );
    _type.m_parents.insert(existing.m_parents.begin(), existing.m_parents.end() );

    by_hash().insert_or_assign(_type.hash_code(), _type);
}

void type_register::log_statistics()
{
    LOG_MESSAGE("reflection", "Logging reflected types ...\n");
    LOG_MESSAGE("reflection", " %-16s %-25s %-60s\n", "-- type hash --", "-- user name --", "-- compiler name --" )

    for ( const auto& [type_hash, type] : by_hash() )
    {
        LOG_MESSAGE("reflection", " %-16llu %-25s %-60s\n", type_hash, type.m_name.c_str(), type.m_compiler_name.c_str() );
    }

    LOG_MESSAGE("reflection", "Logging done.\n");
}
