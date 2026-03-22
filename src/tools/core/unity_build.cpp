#pragma once
#include "reflection/qword.cpp"
#include "reflection/Type.cpp"
#include "reflection/TypeRegister.cpp"
#include "reflection/variant.cpp"
#include "EventManager.cpp"
#include "FileSystem.cpp"
#include "format.cpp"
#include "log.cpp"
#include "StateMachine.cpp"
#include "System.cpp"
#include "TaskManager.cpp"

namespace tools
{
    static void init_reflection()
    {
        DEFINE_REFLECT(double);
        DEFINE_REFLECT(bool);
        DEFINE_REFLECT(void);
        DEFINE_REFLECT(void*);
        DEFINE_REFLECT(any);
        DEFINE_REFLECT(null);
    
        DEFINE_REFLECT_WITH_ALIAS(std::string, "string");
        DEFINE_REFLECT_WITH_ALIAS(i8_t       , "i8");
        DEFINE_REFLECT_WITH_ALIAS(i16_t      , "i16");
        DEFINE_REFLECT_WITH_ALIAS(i32_t      , "i32");
        DEFINE_REFLECT_WITH_ALIAS(i64_t      , "i64");
    } 
}
