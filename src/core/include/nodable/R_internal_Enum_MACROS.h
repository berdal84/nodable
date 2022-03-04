#pragma once

#define R_ENUM( Enum ) \
    static const char* to_string(Enum value) \
    {\
        using type = Enum; \
        switch( value )\
        {

#define R_ENUM_VALUE(VALUE) \
            case type::VALUE: return #VALUE;

#define R_ENUM_END \
            default: return "<not reflected>";\
        } \
    }
