#pragma once

// Header to provide a macro to mimic the #place feature from Jai language
// This feature allows to declare a member at a given location on existing data

#define GETTERS_dynamic_cast( Type, getter_name, target_member ) \
    inline const Type getter_name() const { return dynamic_cast<const Type>(target_member); } \
    inline Type       getter_name()       { return dynamic_cast<Type>(target_member); }

#define GETTERS_static_cast( Type, getter_name, target_member ) \
    inline const Type getter_name() const { return static_cast<const Type>(target_member); } \
    inline Type       getter_name()       { return static_cast<Type>(target_member); }

#define GETTERS_reinterpret_cast( Type, getter_name, target_member ) \
    inline const Type getter_name() const { return reinterpret_cast<const Type>(target_member); } \
    inline Type       getter_name()       { return reinterpret_cast<Type>(target_member); }

#define GETTER( Type, getter_name, target_member ) \
    inline const Type getter_name() const { return target_member; } \
    inline Type       getter_name()       { return target_member; }

#define SETTER( Type, setter_name, target_member ) \
    inline void setter_name(Type value) { target_member = value; }
    