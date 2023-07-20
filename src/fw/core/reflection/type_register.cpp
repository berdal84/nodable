#include "type_register.h"
#include "type.h"
#include "core/log.h"
#include "core/assertions.h"

using namespace fw;

const type* type_register::get(std::size_t index)
{
    auto found = by_index().find(index);
    FW_EXPECT(found != by_index().end(), "reflection: type not found!")
    return found->second;
}

std::unordered_map<std::size_t, const type*>& type_register::by_index()
{
    static std::unordered_map<std::size_t, const type*> meta_type_register_by_typeid;
    return meta_type_register_by_typeid;
}


bool type_register::has(type _type)
{
    return by_index().find(_type.index()) != by_index().end();
}

bool type_register::has(std::size_t index)
{
    auto found = by_index().find(index);
    return found != by_index().end();
}

void type_register::insert(type* _type)
{
    // insert if absent from register
    if( !has(_type->index()))
    {
        by_index().insert({_type->index(), _type});
        return;
    }

    // merge with existing
    const type* existing = get(_type->index());

    LOG_MESSAGE("reflection", "Merge existing: \"%s\" (%s), with: \"%s\" (%s)\n"
    , existing->m_name, existing->m_compiler_name
    , _type->m_name, _type->m_compiler_name
    )
    if( _type->m_name[0] == '\0' )
    {
        _type->m_name = existing->m_name;
    }
    _type->m_children.insert(existing->m_children.begin(), existing->m_children.end() );
    _type->m_parents.insert(existing->m_parents.begin(), existing->m_parents.end() );

    by_index().insert_or_assign(_type->index(), _type);
}

void type_register::log_statistics()
{
    LOG_MESSAGE("reflection", "Logging reflected types ...\n");
    LOG_MESSAGE("reflection", " %-16s %-25s %-60s\n", "-- type hash --", "-- user name --", "-- compiler name --" )

    for ( const auto& [type_hash, type] : by_index() )
    {
        LOG_MESSAGE("reflection", " %-16llu %-25s %-60s\n", type_hash, type->m_name, type->m_compiler_name );
    }

    LOG_MESSAGE("reflection", "Logging done.\n");
}
