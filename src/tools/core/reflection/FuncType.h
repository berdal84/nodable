#pragma once

#include <string>
#include <vector>
#include <tuple>
#include <memory>
#include "type.h"

namespace tools
{
    // forward declarations
    class Operator;

    /*
     * Simple object to store a named function argument
     */
    struct FuncArg
    {
        FuncArg(u8_t index, const type* _type, bool _by_reference, const std::string& _name)
            : m_index( index)
            , m_type(_type)
            , m_by_reference(_by_reference)
            , m_name(_name)
        {
        }

        u8_t        m_index;
        const type* m_type;
        bool        m_by_reference;
        std::string m_name;
    };

    /*
     * Class to store a function signature.
     * We can check if two function signature are matching using this->match(other)
     */
    class FuncType
    {
    public:
        void                           set_identifier(const std::string& _identifier);
        void                           push_arg(const type* _type, bool _by_reference = false);
        bool                           has_an_arg_of_type(const type* type)const;
        bool                           is_exactly(const FuncType* _other)const;
        bool                           is_compatible(const FuncType* _other)const;
        const char*                    get_identifier()const { return m_identifier.c_str(); };
        const FuncArg&                 get_arg(size_t i) const { return m_args[i]; }
        std::vector<FuncArg>&          get_args() { return m_args;};
        const std::vector<FuncArg>&    get_args()const { return m_args;};
        size_t                         get_arg_count() const { return m_args.size(); }
        const type*                    get_return_type() const { return m_return_type; }
        void                           set_return_type(const type* _type) { m_return_type = _type; };
    private:
        tools::string64      m_identifier;
        std::vector<FuncArg> m_args;
        const type*          m_return_type = type::null();

    public:

        /** Push Arg helpers */

        template<class Tuple, std::size_t N> // push N+1 arguments
        struct arg_pusher
        {
            static void push_into(FuncType *_signature)
            {
                arg_pusher<Tuple, N - 1>::push_into(_signature);

                using T = std::tuple_element_t<N-1, Tuple>;
                _signature->push_arg( type::get<T>(), std::is_reference<T>::value );
            }
        };

        template<class Tuple>  // push 1 arguments
        struct arg_pusher<Tuple, 1>
        {
            static void push_into(FuncType *_signature)
            {
                using T = std::tuple_element_t<0, Tuple>;
                _signature->push_arg( type::get<T>(), std::is_reference<T>::value );
            };
        };

        // create an argument_pusher and push arguments into signature
        template<typename... Args, std::enable_if_t<std::tuple_size_v<Args...> != 0, int> = 0>
        void push_args()
        {
            arg_pusher<Args..., std::tuple_size_v<Args...>>::push_into(this);
        }

        // empty function when pushing an empty arguments
        template<typename... Args, std::enable_if_t<std::tuple_size_v<Args...> == 0, int> = 0>
        void push_args(){}
    };

    template<typename T>
    struct FuncTypeBuilder;

    /**
     * Builder to create function/operator signatures for a given language
     * @tparam T is the function's return type
     * @tparam Args is the function's argument(s) type
     *
     * usage: auto* sig = FuncTypeBuilder<double(double,double)>("+");
     */
    template<typename T, typename ...Args>
    struct FuncTypeBuilder<T(Args...)>
    {
        std::string m_id;

        FuncTypeBuilder(const char* id)
        : m_id(id)
        {
            VERIFY(!m_id.empty(), "No identifier specified! use with_id()" );
        }

        FuncType* decorate(FuncType* type)
        {
            type->set_identifier(m_id);
            type->set_return_type(type::get<T>());
            type->push_args<std::tuple<Args...>>();
            return type;
        }

        FuncType* make_instance()
        {
            auto* type = new FuncType();
            decorate(type);
            return type;
        }

        FuncType construct()
        {
            FuncType type;
            decorate(&type);
            return std::move(type);
        }

    };

    template<typename T, typename C, typename ...Args>
    struct FuncTypeBuilder<T(C::*)(Args...)> : FuncTypeBuilder<T(Args...)> {};


}