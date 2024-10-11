#pragma once
#include "Invokable.h"
#include "FuncType.h"
#include "Type.h"
#include "TypeRegister.h"
#include <vector>

namespace tools
{
    // Forward declarations
    template<typename T> class  InvokableStaticFunction;

    namespace type
    {
        /**
         * To reflect a type statically
         * @tparam T the type to reflect
         */
        template<typename T, bool = std::is_class_v<T> >
        struct Initializer;

        // Default implementation
        template<typename T>
        struct Initializer<T, false>
        {
            static_assert(!std::is_class_v<T>);
            TypeDesc *m_type;

            explicit Initializer(const char *_name)
            {
                TypeDesc *type = type::create<T>(_name);
                m_type = TypeRegister::insert_or_merge(type);
            }
        };

        // Class implementation
        template<typename T>
        struct Initializer<T, true>
        {
            static_assert(std::is_class_v<T>);
            ClassDesc *m_class;

            explicit Initializer(const char *_name)
            {
                TypeDesc *type = type::create<T>(_name);
                m_class = (ClassDesc *) TypeRegister::insert_or_merge(type);
            }

            template<typename F>
            Initializer &add_method(F *func_ptr, const char *_name, const char *_alt_name = "")
            {
                FuncTypeBuilder<F> builder{_name};
                auto *invokable = new InvokableStaticFunction<F>(builder.make_instance(), func_ptr);

                m_class->add_static(_name, invokable);

                if (_alt_name[0] != '\0')
                    m_class->add_static(_alt_name, invokable);

                return *this;
            }

            template<typename R, typename C, typename ...Ts>
            Initializer &add_method(R(C::*func_ptr)(Ts...), const char *_name) // non static
            {
                FuncTypeBuilder<R(C::*)(Ts...)> builder{_name};
                auto *invokable = new InvokableMethod<R(C::*)(Ts...)>(builder.make_instance(), func_ptr);
                m_class->add_method(_name, invokable);
                return *this;
            }

            template<typename BaseClassT>
            Initializer &extends()
            {
                static_assert(std::is_base_of_v<BaseClassT, T>);

                auto base_class = const_cast<ClassDesc *>( type::get_class<BaseClassT>()); // get or create
                m_class->add_parent(base_class->id());
                base_class->add_child(m_class->id());
                return *this;
            }
        };
    }
} // namespace headless

#define CAT_IMPL(a, b) a##b
#define CAT(a, b) CAT_IMPL(a, b)

#define REFLECT_STATIC_INIT\
    static void auto_static_initializer();\
namespace /* using the same trick as rttr to avoid name conflicts*/\
{\
    struct auto_static_initializer_struct\
    {\
        auto_static_initializer_struct()\
        {\
            auto_static_initializer();\
        }\
    };\
}\
static const auto_static_initializer_struct CAT(auto_static_initializer, __LINE__);\
static void auto_static_initializer()
