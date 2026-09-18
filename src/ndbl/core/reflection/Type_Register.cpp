#include "Type_Register.h"
#include "Type_Descriptor.h"
#include "ndbl/core/Log.h"
#include "ndbl/core/Asserts.h"

namespace ndbl
{

static std::unordered_map<std::type_index, Type_Descriptor*> g_type_register;

void type_register_clear()
{
    g_type_register.clear();
}

Type_Descriptor* type_register_get(std::type_index index)
{
    auto found = g_type_register.find(index);
    VERIFY(found != g_type_register.end(), "reflection: type not found!");
    return found->second;
}

bool type_register_has(const Type_Descriptor* type)
{
    return g_type_register.find(type->id) != g_type_register.end();
}

bool type_register_has(std::type_index index)
{
    auto found = g_type_register.find(index);
    return found != g_type_register.end();
}

Type_Descriptor* type_register_insert(Type_Descriptor* type)
{
    g_type_register.insert({type->id, type});
    return type;
}

Type_Descriptor* type_register_merge(Type_Descriptor* existing_type, const Type_Descriptor* other_type)
{
    NDBL_DEBUG_LOG(
        Verbosity_Diagnostic,
        __FILE__,
        "Merge existing: \"%s\" (%s), with: \"%s\" (%s)\n",
        existing_type->name.c_str(), existing_type->compiler_name.c_str(),
        other_type->name.c_str(), other_type->compiler_name.c_str()
    );

    if( existing_type->name[0] == '\0' )
    {
        existing_type->name = other_type->name;
    }

    if( existing_type->is_class() )
    {
        existing_type->clss.children.insert(other_type->clss.children.begin(), other_type->clss.children.end() );
        existing_type->clss.parents.insert(other_type->clss.parents.begin(), other_type->clss.parents.end() );
    }

    return existing_type;
}

void type_register_log_statistics()
{
    NDBL_LOG(Verbosity_Diagnostic, "reflection", "Logging reflected types ...\n");
    NDBL_LOG(Verbosity_Diagnostic, "reflection", " %-16s %-25s %-60s\n", "-- type hash --", "-- user name --", "-- compiler name --" );

    for ( const auto& [type_hash, type] : g_type_register )
    {
        NDBL_LOG(Verbosity_Diagnostic, "reflection", " %-16llu %-25s %-60s\n", type_hash, type->name.c_str(), type->compiler_name.c_str() );
    }

    NDBL_LOG(Verbosity_Diagnostic, "reflection", "Logging done.\n");
}

Type_Descriptor* type_register_insert_or_merge(Type_Descriptor* possibly_registered_type)
{
    if( type_register_has(possibly_registered_type->id) )
    {
        Type_Descriptor* existing_type = type_register_get(possibly_registered_type->id);
        return type_register_merge(existing_type, possibly_registered_type);
    }
    return type_register_insert(possibly_registered_type);
}

} // namespace ndbl