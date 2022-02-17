#pragma once

namespace Nodable::R
{
    /**
     * Meta structures to describe a type and get its information at runtime.
     */
    enum class Type : unsigned short int
    {
        Null         = 0,

        // "pure" types:
        Void         = 1 << 0,
        Boolean      = 1 << 1,
        Double       = 1 << 2,
        String       = 1 << 3,

        // qualifiers  (type&&, type&, type*, type**)
        Ref          = 1 << 4,
        DblRef       = 1 << 5 | Ref,
        Ptr          = 1 << 6,
        DblPtr       = 1 << 7 | Ptr,

        Unknown      = Void | DblRef | Ref | Ptr | DblPtr | Boolean | Double | String,

        Void_Ptr     = Void | Ptr,


        COUNT,

        PureTypes_BinaryMask = Void | Boolean | Double | String,
    };

    /* perform a binary and */
    static constexpr Type binary_and(Type _type, Type _right)
    {
        using T = std::underlying_type_t<Type>;
        Type result = (Type)( ((T)_type) & ((T)_right) );
        return result;
    }

    /* perform a binary or */
    static constexpr Type binary_or(Type _type, Type _right)
    {
        using T = std::underlying_type_t<Type>;
        Type result = (Type)( ((T)_type) | ((T)_right) );
        return result;
    }

    static constexpr bool is_ptr(Type left)
    {
        return binary_and(left, R::Type::Ptr) != Type::Null;
    }

    static  constexpr bool is_ref(Type left)
    {
        return binary_and(left, R::Type::Ref) != Type::Null;
    }

    static constexpr Type add_ref(Type _type)
    {
        return binary_or(_type, Type::Ref);
    }

    static constexpr Type add_ptr(Type _type)
    {
        return binary_or(_type, is_ptr(_type) ? Type::DblPtr : Type::Ptr );
    }

    template<typename T>
    struct cpp; // <--------------------{ to convert a cpp type to a Reflect::Type

    template<Type T>
    struct type; // <----------------{ to convert a Reflect::Type type to a cpp type

    template<typename T, Type REFLECT_T> \
    struct meta_info;

    static const char* to_string( Type _type )
    {
        switch( R::binary_and(_type, Type::PureTypes_BinaryMask) )
        {
            case Type::Void:    {return "void";}
            case Type::String:  {return "string";}
            case Type::Double:  {return "double";}
            case Type::Boolean: {return "boolean";}
            default:			{return "unknown";}
        }
    }

    static constexpr bool is_convertible( Type left, Type right )
    {
        Type left_masked  = binary_and(left, right);      // find common bits
        Type result       = binary_and(left_masked, Type::PureTypes_BinaryMask); // keep only  pure type bits
        return result != Type::Null;
    }
}
