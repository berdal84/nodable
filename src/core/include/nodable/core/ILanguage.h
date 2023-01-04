#pragma once

// std
#include <map>
#include <unordered_map>
#include <functional>
#include <tuple>
#include <vector>
#include <regex>
#include <memory> // std::shared_ptr
#include <string>

// Nodable
#include <nodable/core/types.h>
#include <nodable/core/Token_t.h>
#include <nodable/core/Operator_t.h>
#include <nodable/core/reflection/reflection>

namespace ndbl
{
    class Operator;
    class IParser;
    class ISerializer;
    class iinvokable;
    class func_type;

	/**
	 * @brief The role of this class is to define a base abstract class for all languages.
	 *
	 * Using this class we can:
	 * - serialize tokens, functions (calls and signatures) an operators (binary and unary).
	 * - convert TokenType (abstract) to Type (Nodable types)
	 * - create, add and find functions.
	 * - create, add and find operators.
	 * - etc.
	 */
	class ILanguage {
	public:
        using InvokableFunctions_t = std::vector<std::shared_ptr<const iinvokable>>;
        virtual ~ILanguage() = default;

        /** Find a function knowing its type */
        virtual std::shared_ptr<const iinvokable> find_function(const func_type*) const = 0;
        /** Find an operator's function knowing its type (not strict, allow cast)*/
        virtual std::shared_ptr<const iinvokable> find_operator_fct(const func_type*) const = 0;
        /** Find an operator's function knowing its type (strict match only) */
        virtual std::shared_ptr<const iinvokable> find_operator_fct_exact(const func_type*) const = 0;
        /** Find an operator (!= operator's function) from a given symbol (ex: <,>,=,==,!=,...) and type (unary, binary, ternary) */
        virtual const Operator*                 find_operator(const std::string& , Operator_t) const = 0;
        virtual IParser&                        get_parser() = 0;
        virtual const IParser&                  get_parser()const = 0;
        virtual const ISerializer&              get_serializer()const = 0;
        virtual const InvokableFunctions_t&     get_api()const  = 0;
        virtual const std::vector<std::regex>&  get_token_type_regex()const  = 0;
        virtual std::string&                    to_string(std::string&, type)const = 0;
        virtual std::string&                    to_string(std::string&, Token_t)const = 0;
        virtual std::string                     to_string(type)const = 0;
        virtual std::string                     to_string(Token_t)const = 0;
        virtual int                             get_precedence(const iinvokable*)const = 0;
        virtual type                            get_type(Token_t _token)const = 0;
        virtual const std::vector<Token_t>&     get_token_type_regex_index_to_token_type()const = 0;
        virtual void                            add_invokable(std::shared_ptr<const iinvokable>) = 0;
    };
}
