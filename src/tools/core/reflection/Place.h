#pragma once

// Header to provide a macro to mimic the #place feature from Jai language
// This feature allows to declare a member at a given location on existing data

#define PLACE_CAST( Type, alias_name, cast_type, target_member_name ) \
    inline const Type alias_name() const { return cast_type<const Type>(this->target_member_name); } \
    inline Type       alias_name()       { return cast_type<Type>(this->target_member_name); }

#define PLACE( Type, alias_name, target_member_name ) \
    inline const Type alias_name() const { return this->target_member_name; } \
    inline Type       alias_name()       { return this->target_member_name; }