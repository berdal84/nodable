#pragma once

#include <string>
#include <vector>
#include <tuple>
#include <memory>

#include <nodable/core/reflection/R.h>

namespace Nodable
{


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
    class FuncSig
    {
    public:
        enum class Type {
            Function,
            Operator
        };

        FuncSig(Type _type, std::string _identifier, std::string _label = "");
        ~FuncSig() {};
        void                           push_arg(R::MetaType_const_ptr _type, std::string _name = "");

        template <typename... T>
        void push_args(T&&... args) {
            int dummy[] = { 0, ((void) push_arg(std::forward<T>(args)),0)... };
        }

        bool                           has_an_arg_of_type(R::MetaType_const_ptr type)const;
        bool                           is_exactly(const FuncSig* _other)const;
        bool                           is_compatible(const FuncSig* _other)const;
        const std::string&             get_identifier()const { return m_identifier; };
        FuncArgs&                      get_args() { return m_args;};
        const FuncArgs&                get_args()const { return m_args;};
        size_t                         get_arg_count() const { return m_args.size(); }
        const R::MetaType_const_ptr    get_return_type() const { return m_return_type; }
        void                           set_return_type(R::MetaType_const_ptr _type) { m_return_type = _type; };
        std::string                    get_label() const { return m_label; }
        Type                           get_type() const { return m_type; }

    private:
        Type         m_type;
        std::string  m_label;
        std::string  m_identifier;
        FuncArgs     m_args;
        R::MetaType_const_ptr m_return_type;

    public:

        /** helpers to create a FunctionSignature */

        template<typename T>
        struct new_instance;

        template<typename T, typename... Args>
        struct new_instance<T(Args...)>
        {
            using F = T(Args...);
            static FuncSig* with_id(FuncSig::Type _type, const char* _identifier, const char* _label = "")
            {
                auto signature = new FuncSig(_type, _identifier, _label);
                signature->set_return_type(R::get_meta_type<T>() );
                signature->push_args<std::tuple<Args...>>();
                return signature;
            }
        };

        /** Push Arg helpers */

        template<class Tuple, std::size_t N> // push N+1 arguments
        struct arg_pusher
        {
            static void push_into(FuncSig *_signature)
            {
                arg_pusher<Tuple, N - 1>::push_into(_signature);

                using T = std::tuple_element_t<N-1, Tuple>;
                _signature->push_arg(R::get_meta_type<T>() );
            }
        };

        template<class Tuple>  // push 1 arguments
        struct arg_pusher<Tuple, 1>
        {
            static void push_into(FuncSig *_signature)
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