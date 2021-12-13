#pragma once
namespace Nodable
{
	/*
		The role of this enum class is to distinguish between all types that Nodable can handle.
	*/
	enum Type
	{
        Type_Unknown      = 0,
        Type_Pointer      = 0b00000001, // 1
        Type_Reference    = 0b00000010, // 2
                                        // 3 (for future use)
        Type_Boolean      = 0b00001000, // 8
        Type_Double       = 0b00010000, // 16
        Type_String       = 0b00100000, // 32
        Type_Any          = Type_Boolean | Type_Double | Type_String,

        Type_COUNT        = 0b01000000,

        Type_Boolean_Ptr  = Type_Boolean | Type_Pointer,
        Type_Double_Ptr   = Type_Double | Type_Pointer,
        Type_String_Ptr   = Type_String | Type_Pointer,
        Type_Any_Ptr      = Type_Any | Type_Pointer,

        Type_Boolean_Ref  = Type_Boolean | Type_Reference,
        Type_Double_Ref   = Type_Double | Type_Reference,
        Type_String_Ref   = Type_String | Type_Reference,
        Type_Any_Ref      = Type_Any | Type_Reference,
    };


    inline static bool is_convertible( Type left, Type right )
    {
        return (( left & Type_Any ) & ( right & Type_Any )) != 0; // check if type are matching (ignoring ref/pointer)
    }

    inline static bool is_pointer( Type left)
    {
        return (left & Type_Pointer ) != 0;
    }

    inline static bool is_reference( Type left)
    {
        return (left & Type_Reference ) != 0;
    }

    /*
        The role of these templates is to help conversion between cpp type to nodable Type

        double <--> Type_Double
        bool   <--> Type_Boolean
        ...
    */
    template<typename T>
    struct to_Type;

    template<Type T>
    struct from_Type;

#define DECL( cpp_type, nodable_type ) \
    template<> \
    struct to_Type<cpp_type> { \
        static constexpr Type type = nodable_type; \
        static constexpr const char* type_name = #nodable_type; \
        static constexpr const char* cpp_type_name = #cpp_type; \
    }; \
    template<> \
    struct from_Type<nodable_type> { \
        using type = cpp_type; \
    };\
    template<> \
    struct from_Type<nodable_type##_Ptr> { \
        using type = cpp_type*; \
    };\
    template<> \
    struct from_Type<nodable_type##_Ref> { \
        using type = cpp_type&; \
    };\
    template<> \
    struct to_Type<cpp_type&> { \
        static constexpr Type type = (Type)(nodable_type | Type_Reference); \
        static constexpr const char* type_name = #nodable_type"&"; \
        static constexpr const char* cpp_type_name = #cpp_type"&"; \
    }; \
    template<> \
    struct to_Type<cpp_type*> { \
        static constexpr Type type = (Type)(nodable_type | Type_Pointer); \
        static constexpr const char* type_name = #nodable_type"*"; \
        static constexpr const char* cpp_type_name = #cpp_type"*"; \
    };


    DECL( double, Type_Double )
    DECL(std::string  , Type_String )
    DECL( bool, Type_Boolean )
#undef DECL
}