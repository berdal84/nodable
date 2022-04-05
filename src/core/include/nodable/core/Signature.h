#pragma once

#include <string>
#include <vector>
#include <tuple>
#include <memory>

#include <nodable/core/reflection/R.h>
#include <nodable/core/constants.h>
#include <regex>
#include "Operator.h"

namespace Nodable
{
    // forward declarations
    class Operator;

    /*
     * Simple object to store a named function argument
     */
    struct FuncArg
    {
        FuncArg(R::MetaType_const_ptr _type, std::string& _name): m_type(_type), m_name(_name){}
        R::MetaType_const_ptr m_type;
        std::string           m_name;
    };

    using FuncArgs = std::vector<FuncArg>;

    /*
     * Class to store a function signature.
     * We can check if two function signature are matching using this->match(other)
     */
    class Signature
    {
        using Meta_t = std::shared_ptr<const R::MetaType>;

    public:
        enum class Type {
            Function,
            Operator
        };

        Signature(std::string _id);

        Signature(const Operator* _op);

        ~Signature() {};
        void                           push_arg(R::MetaType_const_ptr _type);

        template <typename... T>
        void push_args(T&&... args) {
            int dummy[] = { 0, ((void) push_arg(std::forward<T>(args)),0)... };
        }

        bool                           has_an_arg_of_type(R::MetaType_const_ptr type)const;
        bool                           is_exactly(const Signature* _other)const;
        bool                           is_compatible(const Signature* _other)const;
        bool                           is_operator()const { return m_operator; };
        const std::string&             get_identifier()const { return m_identifier; };
        FuncArgs&                      get_args() { return m_args;};
        const FuncArgs&                get_args()const { return m_args;};
        size_t                         get_arg_count() const { return m_args.size(); }
        const R::MetaType_const_ptr    get_return_type() const { return m_return_type; }
        void                           set_return_type(R::MetaType_const_ptr _type) { m_return_type = _type; };
        const Operator*                get_operator()const { return m_operator; }
        std::string                    get_label()const;
        static std::string&            clean_function_id(std::string& _id);
        static const Signature*        new_operator(Meta_t, const Operator* _op, Meta_t );
        static const Signature*        new_operator(Meta_t, const Operator* _op, Meta_t , Meta_t );
    private:
        const Operator* m_operator;
        std::string     m_identifier;
        FuncArgs        m_args;
        R::MetaType_const_ptr m_return_type;

    public:

        /** helpers to create a FunctionSignature */

        template<typename T>
        struct from_type;

        template<typename T, typename... Args>
        struct from_type<T(Args...)>
        {
            using F = T(Args...);

            static Signature* as_function(std::string _id)
            {
                auto signature = new Signature(_id);
                signature->set_return_type(R::get_meta_type<T>() );
                signature->push_args<std::tuple<Args...>>();
                return signature;
            }

            static Signature* as_operator(const Operator* _op)
            {
                NODABLE_ASSERT(_op);
                auto signature = new Signature( _op);
                signature->set_return_type(R::get_meta_type<T>() );
                signature->push_args<std::tuple<Args...>>();
                return signature;
            }
        };

        /** Push Arg helpers */

        template<class Tuple, std::size_t N> // push N+1 arguments
        struct arg_pusher
        {
            static void push_into(Signature *_signature)
            {
                arg_pusher<Tuple, N - 1>::push_into(_signature);

                using T = std::tuple_element_t<N-1, Tuple>;
                _signature->push_arg(R::get_meta_type<T>() );
            }
        };

        template<class Tuple>  // push 1 arguments
        struct arg_pusher<Tuple, 1>
        {
            static void push_into(Signature *_signature)
            {
                using T = std::tuple_element_t<0, Tuple>;
                _signature->push_arg(R::get_meta_type<T>() );
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
}