#pragma once
namespace Nodable
{
	/*
		The role of this enum class is to distinguish between all types that Nodable can handle.
	*/
	enum Type
	{
        Type_Unknown = 0,
        Type_Any,
        Type_Boolean,
        Type_Double,
        Type_String,
        // TODO: implement references
        Type_COUNT
	};

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
    };

    DECL( double       , Type_Double )
    DECL( std::string  , Type_String )
    DECL( bool         , Type_Boolean )
#undef DECL
}