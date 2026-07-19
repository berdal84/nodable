#pragma once

// Header to provide a macro to mimic the #place feature from Jai language
// This feature allows to declare a member at a given location on existing data

#define GETTERS_dynamic_cast( Type, alias_name, target_member_name ) \
    inline const Type alias_name() const { return dynamic_cast<const Type>(this->target_member_name); } \
    inline Type       alias_name()       { return dynamic_cast<Type>(this->target_member_name); }

#define GETTERS_static_cast( Type, alias_name, target_member_name ) \
    inline const Type alias_name() const { return static_cast<const Type>(this->target_member_name); } \
    inline Type       alias_name()       { return static_cast<Type>(this->target_member_name); }

#define GETTERS_reinterpret_cast( Type, alias_name, target_member_name ) \
    inline const Type alias_name() const { return reinterpret_cast<const Type>(this->target_member_name); } \
    inline Type       alias_name()       { return reinterpret_cast<Type>(this->target_member_name); }

#define GETTER( Type, alias_name, target_member_name ) \
    inline const Type alias_name() const { return this->target_member_name; } \
    inline Type       alias_name()       { return this->target_member_name; }
    