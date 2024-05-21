#include "type_register.h"
#include "type.h"
#include "fw/core/log.h"
#include "fw/core/assertions.h"

using namespace fw;

type* type_register::get(std::type_index index)
{
    auto found = by_index().find(index);
    FW_EXPECT(found != by_index().end(), "reflection: type not found!")
    return found->second;
}

std::unordered_map<std::type_index, type*>& type_register::by_index()
{
    static std::unordered_map<std::type_index, type*> meta_type_register_by_typeid;
    return meta_type_register_by_typeid;
}


bool type_register::has(type _type)
{
    return by_index().find(_type.id()) != by_index().end();
}

bool type_register::has(std::type_index index)
{
    auto found = by_index().find(index);
    return found != by_index().end();
}

void type_register::insert(type* _type)
{
    by_index().insert({_type->id(), _type});
}

type* type_register::merge(type* existing, const type* other)
{
    LOG_VERBOSE("reflection", "Merge existing: \"%s\" (%s), with: \"%s\" (%s)\n"
    , existing->m_name, existing->m_compiler_name
    , other->m_name, other->m_compiler_name
    )
    if( existing->m_name[0] == '\0' )
    {
        existing->m_name = other->m_name;
    }
    existing->m_children.insert(other->m_children.begin(), other->m_children.end() );
    existing->m_parents.insert(other->m_parents.begin(), other->m_parents.end() );
    return existing;
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

type* type_register::insert_or_merge(type* possibly_existing_type)
{
    if( type_register::has(possibly_existing_type->id()))
    {
        type* existing_type = type_register::get(possibly_existing_type->id());
        return merge(existing_type, possibly_existing_type);
    }
    type_register::insert(possibly_existing_type);
    return possibly_existing_type;
}
