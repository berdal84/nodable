#include "Type_Register.h"
#include "Type_Descriptor.h"
#include "tools/core/Log.h"
#include "tools/core/Asserts.h"

using namespace tools;

Type_Descriptor* Type_Register::get(std::type_index index)
{
    auto found = by_index().find(index);
    VERIFY(found != by_index().end(), "reflection: type not found!");
    return found->second;
}

Class_Descriptor* Type_Register::get_class(std::type_index index)
{
    Type_Descriptor* type = get(index);
    if ( type->is_class() )
        return static_cast<Class_Descriptor*>( type );
    return nullptr;
}

std::unordered_map<std::type_index, Type_Descriptor*>& Type_Register::by_index()
{
    static std::unordered_map<std::type_index, Type_Descriptor*> meta_type_register_by_typeid;
    return meta_type_register_by_typeid;
}


bool Type_Register::has(const Type_Descriptor* _type)
{
    return by_index().find(_type->id()) != by_index().end();
}

bool Type_Register::has(std::type_index index)
{
    auto found = by_index().find(index);
    return found != by_index().end();
}

Type_Descriptor* Type_Register::insert(Type_Descriptor* _type)
{
    by_index().insert({_type->id(), _type});
    return _type;
}

Type_Descriptor* Type_Register::merge(Type_Descriptor* existing, const Type_Descriptor* other)
{
    TOOLS_DEBUG_LOG(
        tools::Verbosity_Diagnostic,
        __FILE__,
        "Merge existing: \"%s\" (%s), with: \"%s\" (%s)\n",
        existing->m_name.c_str(), existing->m_compiler_name.c_str(),
        other->m_name.c_str(), other->m_compiler_name.c_str()
    );

    if( existing->m_name[0] == '\0' )
    {
        existing->m_name = other->m_name;
    }

    if ( existing->is_class() )
    {
        auto* existing_class = reinterpret_cast<Class_Descriptor*>(existing);
        auto* other_class    = reinterpret_cast<const Class_Descriptor*>(other);

        existing_class->m_children.insert(other_class->m_children.begin(), other_class->m_children.end() );
        existing_class->m_parents.insert(other_class->m_parents.begin(), other_class->m_parents.end() );
    }

    return existing;
}

void Type_Register::log_statistics()
{
    TOOLS_LOG(tools::Verbosity_Diagnostic, "reflection", "Logging reflected types ...\n");
    TOOLS_LOG(tools::Verbosity_Diagnostic, "reflection", " %-16s %-25s %-60s\n", "-- type hash --", "-- user name --", "-- compiler name --" );

    for ( const auto& [type_hash, type] : by_index() )
    {
        TOOLS_LOG(tools::Verbosity_Diagnostic, "reflection", " %-16llu %-25s %-60s\n", type_hash, type->m_name.c_str(), type->m_compiler_name.c_str() );
    }

    TOOLS_LOG(tools::Verbosity_Diagnostic, "reflection", "Logging done.\n");
}

Type_Descriptor* Type_Register::insert_or_merge(Type_Descriptor* possibly_existing_type)
{
    if( has(possibly_existing_type->id()) )
    {
        Type_Descriptor* existing_type = get(possibly_existing_type->id());
        return merge(existing_type, possibly_existing_type);
    }
    return insert(possibly_existing_type);
}
