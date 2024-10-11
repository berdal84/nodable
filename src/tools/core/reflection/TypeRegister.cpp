#include "TypeRegister.h"
#include "Type.h"
#include "tools/core/log.h"
#include "tools/core/assertions.h"

using namespace tools;

TypeDesc* TypeRegister::get(std::type_index index)
{
    auto found = by_index().find(index);
    VERIFY(found != by_index().end(), "reflection: type not found!")
    return found->second;
}

ClassDesc* TypeRegister::get_class(std::type_index index)
{
    TypeDesc* type = get(index);
    if ( type->is_class() )
        return static_cast<ClassDesc*>( type );
    return nullptr;
}

std::unordered_map<std::type_index, TypeDesc*>& TypeRegister::by_index()
{
    static std::unordered_map<std::type_index, TypeDesc*> meta_type_register_by_typeid;
    return meta_type_register_by_typeid;
}


bool TypeRegister::has(const TypeDesc* _type)
{
    return by_index().find(_type->id()) != by_index().end();
}

bool TypeRegister::has(std::type_index index)
{
    auto found = by_index().find(index);
    return found != by_index().end();
}

TypeDesc* TypeRegister::insert(TypeDesc* _type)
{
    by_index().insert({_type->id(), _type});
    return _type;
}

TypeDesc* TypeRegister::merge(TypeDesc* existing, const TypeDesc* other)
{
    LOG_VERBOSE(
        __FILE__,
        "Merge existing: \"%s\" (%s), with: \"%s\" (%s)\n",
        existing->m_name, existing->m_compiler_name,
        other->m_name, other->m_compiler_name
    )
    if( existing->m_name[0] == '\0' )
    {
        existing->m_name = other->m_name;
    }

    if ( existing->is_class() )
    {
        auto* existing_class = reinterpret_cast<ClassDesc*>(existing);
        auto* other_class    = reinterpret_cast<const ClassDesc*>(other);

        existing_class->m_children.insert(other_class->m_children.begin(), other_class->m_children.end() );
        existing_class->m_parents.insert(other_class->m_parents.begin(), other_class->m_parents.end() );
    }

    return existing;
}

void TypeRegister::log_statistics()
{
    LOG_MESSAGE("reflection", "Logging reflected types ...\n");
    LOG_MESSAGE("reflection", " %-16s %-25s %-60s\n", "-- type hash --", "-- user name --", "-- compiler name --" )

    for ( const auto& [type_hash, type] : by_index() )
    {
        LOG_MESSAGE("reflection", " %-16llu %-25s %-60s\n", type_hash, type->m_name, type->m_compiler_name );
    }

    LOG_MESSAGE("reflection", "Logging done.\n");
}

TypeDesc* TypeRegister::insert_or_merge(TypeDesc* possibly_existing_type)
{
    if( has(possibly_existing_type->id()) )
    {
        TypeDesc* existing_type = get(possibly_existing_type->id());
        return merge(existing_type, possibly_existing_type);
    }
    return insert(possibly_existing_type);
}
