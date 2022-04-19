#pragma once

#include <nodable/core/Log.h>
#include <nodable/core/ILanguage.h>
#include <nodable/core/Signature.h>
#include <nodable/core/Operator.h>
#include <string>

namespace Nodable
{
    template<typename T>
    struct SignatureBuilder;

    /**
     * Builder to create function/operator signatures for a given language
     * @tparam T is the function's return type
     * @tparam Args is the function's argument(s) type
     *
     * usage: Signature* sig = signature-builder<double(double,double)>::signature()
     *                                                                  .with_id("+")
     *                                                                  .as_operator()
     *                                                                  .with_language(lang_ptr).build();
     */
    template<typename T, typename ...Args>
    struct SignatureBuilder<T(Args...)>
    {
        std::string       m_id;
        const ILanguage*  m_language;
        bool              m_as_operator;

        SignatureBuilder()
                : m_as_operator(false)
                , m_language(nullptr){}

        void with_id(const std::string& _id) { m_id = _id; }
        void with_lang(const ILanguage* _language) { m_language = _language; }
        void as_operator(bool _b = true) { m_as_operator = _b; }

        Signature* construct()
        {
            NODABLE_ASSERT_EX( m_language   , "No language specified! use with_lang()" );
            NODABLE_ASSERT_EX( !m_id.empty(), "No identifier specified! use with_id()" );

            Signature* signature;
            if( m_as_operator )
            {
                const Operator* op;
                size_t argc = sizeof...(Args);
                switch ( argc )
                {
                    case 1:  op = m_language->find_operator(m_id, Operator_t::Unary);  break;
                    case 2:  op = m_language->find_operator(m_id, Operator_t::Binary); break;
                    default: op = nullptr;
                }
                NODABLE_ASSERT_EX( op, "No operator found  in language for specified id!" );
                signature = new Signature(m_language->sanitize_operator_id(op->identifier), op);
            }
            else
            {
                m_id = m_language->sanitize_function_id(m_id);
                NODABLE_ASSERT_EX( !m_id.empty(), "Identifier after sanitization is empty!" );
                signature = new Signature( m_id );
            }

            signature->set_return_type(type::get<T>());
            signature->push_args<std::tuple<Args...>>();

            return signature;
        }

        static Signature* new_function(const std::string& _id, const ILanguage* _language)
        {
            SignatureBuilder<T(Args...)> builder;
            builder.with_id(_id);
            builder.with_lang(_language);
            return builder.construct();
        }

        static Signature* new_operator(const std::string& _id, const ILanguage* _language)
        {
            SignatureBuilder<T(Args...)> builder;
            builder.with_id(_id);
            builder.with_lang(_language);
            builder.as_operator();
            return builder.construct();
        }
    };


}