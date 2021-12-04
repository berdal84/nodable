#pragma once

#include <string>
#include <vector>

#include <nodable/Type.h>

namespace Nodable
{

    /*
     * Simple object to store a named function argument
     */
    struct FunctionArg
    {
        FunctionArg(Type _type, std::string& _name):m_type(_type),m_name(_name){}
        Type        m_type;
        std::string m_name;
    };

    /*
     * Class to store a function signature.
     * We can check if two function signature are matching using this->match(other)
     */
    class FunctionSignature {
    public:
        FunctionSignature(std::string _identifier, Type _type, std::string _label = "");
        ~FunctionSignature() {};
        void                           push_arg(Type _type, std::string _name = "");

        template <typename... Type>
        void push_args(Type &&... args) {
            int dummy[] = { 0, ((void) push_arg(std::forward<Type>(args)),0)... };
        }

        bool                           has_an_arg_of_type(Type type)const;
        bool                           match(const FunctionSignature* _other)const;
        const std::string&             get_identifier()const;
        std::vector<FunctionArg>       get_args() const;
        size_t                         get_arg_count() const { return m_args.size(); }
        Type                           get_return_type() const;
        std::string                    get_label() const;

    private:
        std::string m_label;
        std::string m_identifier;
        Type        m_return_type;
        std::vector<FunctionArg> m_args;

    public:
        template<typename R = Type, typename... Type>
        static FunctionSignature create(R _type, std::string _identifier, Type &&..._args) {
            FunctionSignature signature(_identifier, _type);
            signature.push_args(_args...);
            return signature;
        }
    };
}