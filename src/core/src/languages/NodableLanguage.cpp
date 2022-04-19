#include <nodable/core/languages/NodableLanguage.h>

#include <nodable/core/Member.h>
#include <nodable/core/String.h>
#include <nodable/core/System.h>

// Nodable API begin
#include <nodable/core/languages/lib_math.h>
#include <nodable/core/languages/lib_biology.h>
// Nodable API end

using namespace Nodable;
using namespace Nodable::lib::math;
using namespace Nodable::lib::biology;

using string = std::string;

NodableLanguage::NodableLanguage(): m_parser(*this), m_serializer(*this)
{
    /*
     *  Configure the Semantic.
     *
     *  The order of insertion is important. First inserted will be taken in priority by Parser.
     */

    // ignored
    add_regex(std::regex("^(//(.+?)$)"), Token_t::ignore); // Single line
    add_regex(std::regex("^(/\\*(.+?)\\*/)"), Token_t::ignore); // Multi line

    // keywords
    add_string(k_keyword_if           , Token_t::keyword_if);
    add_string(k_keyword_else         , Token_t::keyword_else);
    add_string(k_keyword_for          , Token_t::keyword_for);
    add_string(k_keyword_operator     , Token_t::keyword_operator);
    add_type(type::get<bool>()        , Token_t::keyword_bool    , k_keyword_bool);
    add_type(type::get<std::string>() , Token_t::keyword_string  , k_keyword_string);
    add_type(type::get<double>()      , Token_t::keyword_double  , k_keyword_double);
    add_type(type::get<i16_t>()       , Token_t::keyword_int     , k_keyword_int);

    // punctuation
    add_char(k_tab                , Token_t::ignore );
    add_char(k_space              , Token_t::ignore);
    add_char(k_open_curly_brace   , Token_t::scope_begin );
    add_char(k_close_curly_brace  , Token_t::scope_end);
    add_char(k_open_bracket       , Token_t::fct_params_begin);
    add_char(k_close_bracket      , Token_t::fct_params_end );
    add_char(k_coma               , Token_t::fct_params_separator);
    add_char(k_semicolon          , Token_t::end_of_instruction);
    add_char(k_end_of_line        , Token_t::end_of_line);

    // literals
    // USE OTHER METHOD: insert(std::regex("^(true|false)")                , Token_t::literal_bool  , type::get<bool>());
    add_regex(std::regex(R"(^("[^"]*"))")                , Token_t::literal_string , type::get<std::string>());
    add_regex(std::regex("^(0|([1-9][0-9]*))(\\.[0-9]+)"), Token_t::literal_double , type::get<double>());
    add_regex(std::regex("^(0|([1-9][0-9]*))")           , Token_t::literal_int    , type::get<i16_t>());

    // identifier
    add_regex(std::regex("^([a-zA-Z_]+[a-zA-Z0-9]*)"), Token_t::identifier);

    // operators
    add_regex(std::regex("^(<=>)"), Token_t::operator_); // 3 chars
    add_regex(std::regex("^([=\\|&]{2}|(<=)|(>=)|(=>)|(!=))"), Token_t::operator_); // 2 chars
    add_regex(std::regex("^[/+\\-*!=<>]"), Token_t::operator_); // single char

    add_operator( "-"  , Operator_t::Unary, 5); // --------- unary (sorted by precedence)
    add_operator( "!"  , Operator_t::Unary, 5);

    add_operator( "/"  , Operator_t::Binary , 20); // ------- binary (sorted by precedence)
    add_operator( "*"  , Operator_t::Binary , 20);
    add_operator( "+"  , Operator_t::Binary , 10);
    add_operator( "-"  , Operator_t::Binary , 10);
    add_operator( "||" , Operator_t::Binary , 10);
    add_operator( "&&" , Operator_t::Binary , 10);
    add_operator( ">=" , Operator_t::Binary , 10);
    add_operator( "<=" , Operator_t::Binary , 10);
    add_operator( "=>" , Operator_t::Binary , 10);
    add_operator( "==" , Operator_t::Binary , 10);
    add_operator( "<=>", Operator_t::Binary , 10);
    add_operator( "!=" , Operator_t::Binary , 10);
    add_operator( ">"  , Operator_t::Binary , 10);
    add_operator( "<"  , Operator_t::Binary , 10);
    add_operator( "="  , Operator_t::Binary , 0);

    // operator implementations
    BIND_OPERATOR_T(api_plus, "+", double(double, i16_t))
    BIND_OPERATOR_T(api_plus, "+", double(double, double))
    BIND_OPERATOR_T(api_plus, "+", i16_t(i16_t, i16_t))
    BIND_OPERATOR_T(api_plus, "+", i16_t(i16_t, double))
    BIND_OPERATOR_T(api_plus, "+", string(string, string))
    BIND_OPERATOR_T(api_plus, "+", string(string, i16_t))
    BIND_OPERATOR_T(api_plus, "+", string(string, double))

    BIND_OPERATOR(api_or, "||")
    BIND_OPERATOR(api_and, "&&")

    BIND_OPERATOR_T(api_minus, "-", double(double))
    BIND_OPERATOR_T(api_minus, "-", i16_t(i16_t))

    BIND_OPERATOR_T(api_minus, "-", double(double, double))
    BIND_OPERATOR_T(api_minus, "-", double(double, i16_t))
    BIND_OPERATOR_T(api_minus, "-", i16_t(i16_t, i16_t))
    BIND_OPERATOR_T(api_minus, "-", i16_t(i16_t, double))

    BIND_OPERATOR_T(api_divide, "/", double(double, double))
    BIND_OPERATOR_T(api_divide, "/", double(double, i16_t))
    BIND_OPERATOR_T(api_divide, "/", i16_t(i16_t, i16_t))
    BIND_OPERATOR_T(api_divide, "/", i16_t(i16_t, double))

    BIND_OPERATOR_T(api_multiply, "*", double(double, double))
    BIND_OPERATOR_T(api_multiply, "*", double(double, i16_t))
    BIND_OPERATOR_T(api_multiply, "*", i16_t(i16_t, i16_t))
    BIND_OPERATOR_T(api_multiply, "*", i16_t(i16_t, double))

    BIND_OPERATOR_T(api_minus, "-", double(double, double))
    BIND_OPERATOR_T(api_minus, "-", double(double, i16_t))
    BIND_OPERATOR_T(api_minus, "-", i16_t(i16_t, i16_t))
    BIND_OPERATOR_T(api_minus, "-", i16_t(i16_t, double))

    BIND_OPERATOR_T(api_greater_or_eq, ">=", bool(double, double))
    BIND_OPERATOR_T(api_greater_or_eq, ">=", bool(double, i16_t))
    BIND_OPERATOR_T(api_greater_or_eq, ">=", bool(i16_t, double))
    BIND_OPERATOR_T(api_greater_or_eq, ">=", bool(i16_t, i16_t))

    BIND_OPERATOR_T(api_lower_or_eq, "<=", bool(double, i16_t))
    BIND_OPERATOR_T(api_lower_or_eq, "<=", bool(double, double))
    BIND_OPERATOR_T(api_lower_or_eq, "<=", bool(i16_t, i16_t))
    BIND_OPERATOR_T(api_lower_or_eq, "<=", bool(i16_t, double))

    BIND_OPERATOR(api_not, "!")

    BIND_OPERATOR_T(api_assign, "=", string(string & , string))
    BIND_OPERATOR_T(api_assign, "=", bool(bool & , bool))
    BIND_OPERATOR_T(api_assign, "=", double(double & , i16_t))
    BIND_OPERATOR_T(api_assign, "=", double(double & , double))
    BIND_OPERATOR_T(api_assign, "=", i16_t(i16_t & , i16_t))
    BIND_OPERATOR_T(api_assign, "=", i16_t(i16_t & , double))

    BIND_OPERATOR(api_implies, "=>")

    BIND_OPERATOR_T(api_equals, "==", bool(i16_t, i16_t))
    BIND_OPERATOR_T(api_equals, "==", bool(double, double))
    BIND_OPERATOR_T(api_equals, "==", bool(string, string))

    BIND_OPERATOR_T(api_equals, "<=>", bool(bool, bool))

    BIND_OPERATOR_T(api_not_equals, "!=", bool(bool, bool))
    BIND_OPERATOR_T(api_not_equals, "!=", bool(i16_t, i16_t))
    BIND_OPERATOR_T(api_not_equals, "!=", bool(double, double))
    BIND_OPERATOR_T(api_not_equals, "!=", bool(string, string))

    BIND_OPERATOR_T(api_greater, ">", bool(double, double))
    BIND_OPERATOR_T(api_greater, ">", bool(i16_t, i16_t))

    BIND_OPERATOR_T(api_lower, "<", bool(double, double))
    BIND_OPERATOR_T(api_lower, "<", bool(i16_t, i16_t))

    // functions

    BIND_FUNCTION_T(api_return, bool(bool))
    BIND_FUNCTION_T(api_return, i16_t(i16_t))
    BIND_FUNCTION_T(api_return, double(double))
    BIND_FUNCTION_T(api_return, string(string))

    BIND_FUNCTION(api_sin)
    BIND_FUNCTION(api_cos)
    BIND_FUNCTION_T(api_plus, i16_t(i16_t, i16_t))
    BIND_FUNCTION_T(api_minus, i16_t(i16_t, i16_t))
    BIND_FUNCTION_T(api_multiply, i16_t(i16_t, i16_t))
    BIND_FUNCTION_T(api_plus, double(double, double))
    BIND_FUNCTION_T(api_minus, double(double, double))
    BIND_FUNCTION_T(api_multiply, double(double, double))
    BIND_FUNCTION_T(api_sqrt, double(double))
    BIND_FUNCTION_T(api_sqrt, i16_t(i16_t))
    BIND_FUNCTION(api_not)
    BIND_FUNCTION(api_or)
    BIND_FUNCTION(api_and)
    BIND_FUNCTION(api_xor)
    BIND_FUNCTION(api_to_bool)
    BIND_FUNCTION(api_mod)
    BIND_FUNCTION_T(api_pow, i16_t(i16_t, i16_t))
    BIND_FUNCTION_T(api_pow, double(double, double))
    BIND_FUNCTION(api_secondDegreePolynomial)
    BIND_FUNCTION(api_DNAtoProtein)
    BIND_FUNCTION_T(api_to_string, string(bool))
    BIND_FUNCTION_T(api_to_string, string(double))
    BIND_FUNCTION_T(api_to_string, string(i16_t))
    BIND_FUNCTION_T(api_to_string, string(string))

    BIND_FUNCTION_T(api_print, string(bool))
    BIND_FUNCTION_T(api_print, string(double))
    BIND_FUNCTION_T(api_print, string(i16_t))
    BIND_FUNCTION_T(api_print, string(string))
}

std::string NodableLanguage::sanitize_function_id(const std::string &_id) const
{
    if( _id.compare(0, 4, "api_") == 0)
    {
        return _id.substr(4);
    }
    return _id;
}

std::string NodableLanguage::sanitize_operator_id(const std::string &_id) const
{
    return k_keyword_operator + _id;
}

#include <nodable/core/ILanguage.h>
#include <vector>
#include <nodable/core/Node.h>
#include <nodable/core/languages/NodableParser.h>
#include <nodable/core/Operator.h>
#include <nodable/core/IParser.h>
#include <nodable/core/ISerializer.h>

using namespace Nodable;

NodableLanguage::~NodableLanguage()
{
    for( auto each : m_operators ) delete each;
    for( auto each : m_functions ) delete each;
    // for( auto each : m_operator_implems ) delete each; (duplicates from m_functions)
}

const IInvokable* NodableLanguage::find_function(const Signature* _signature) const
{
    auto is_compatible = [&](const IInvokable* fct)
    {
        return fct->get_signature()->is_compatible(_signature);
    };

    auto it = std::find_if(m_functions.begin(), m_functions.end(), is_compatible);

    if (it != m_functions.end())
    {
        return *it;
    }

    return nullptr;
}

const IInvokable* NodableLanguage::find_operator_fct_exact(const Signature* _signature) const
{
    if(!_signature)
    {
        return nullptr;
    }

    auto is_exactly = [&](const IInvokable* _invokable)
    {
        return _signature->is_exactly(_invokable->get_signature());
    };

    auto found = std::find_if(m_operator_implems.cbegin(), m_operator_implems.cend(), is_exactly );

    if (found != m_operator_implems.end() )
    {
        return *found;
    }

    return nullptr;
}

const IInvokable* NodableLanguage::find_operator_fct(const Signature* _signature) const
{
    if(!_signature)
    {
        return nullptr;
    }
    auto exact = find_operator_fct_exact(_signature);
    if( !exact) return find_operator_fct_fallback(_signature);
    return exact;
}

const IInvokable* NodableLanguage::find_operator_fct_fallback(const Signature* _signature) const
{

    auto is_compatible = [&](const IInvokable* _invokable)
    {
        return _signature->is_compatible(_invokable->get_signature());
    };

    auto found = std::find_if(m_operator_implems.cbegin(), m_operator_implems.cend(), is_compatible );

    if (found != m_operator_implems.end() )
    {
        return *found;
    }

    return nullptr;
}


void NodableLanguage::add_invokable(const IInvokable* _invokable)
{
    m_functions.push_back(_invokable);

    std::string signature;
    m_serializer.serialize(signature, _invokable->get_signature() );

    if( _invokable->get_signature()->is_operator() )
    {
        auto found = std::find(m_operator_implems.begin(), m_operator_implems.end(), _invokable);
        NODABLE_ASSERT( found == m_operator_implems.end() )
        m_operator_implems.push_back(_invokable);

        LOG_VERBOSE("Language", "%s added to functions and operator implems\n", signature.c_str() );
    }
    else
    {
        LOG_VERBOSE("Language", "%s added to functions\n", signature.c_str() );
    }
}

void NodableLanguage::add_operator(const char* _id, Operator_t _type, int _precedence)
{
    const Operator* op = new Operator(_id, _type, _precedence);

    NODABLE_ASSERT( std::find( m_operators.begin(), m_operators.end(), op) == m_operators.end() )
    m_operators.push_back(op);
}

const Operator* NodableLanguage::find_operator(const std::string& _identifier, Operator_t _type) const
{
    auto is_exactly = [&](const Operator* op)
    {
        return op->identifier == _identifier && op->type == _type;
    };

    auto found = std::find_if(m_operators.cbegin(), m_operators.cend(), is_exactly );

    if (found != m_operators.end() )
        return *found;

    return nullptr;
}


void NodableLanguage::add_regex(const std::regex& _regex, Token_t _token_t)
{
    m_token_regex.push_back(_regex);
    m_regex_to_token.push_back(_token_t);
}

void NodableLanguage::add_regex(const std::regex& _regex, Token_t _token_t, type _type)
{
    m_token_regex.push_back(_regex);
    m_regex_to_token.push_back(_token_t);

    m_type_regex.push_back(_regex);
    m_regex_to_type.push_back(_type);

    m_token_to_type.insert({_token_t, _type});
    m_type_to_token.insert({_type.hash_code(), _token_t});
}

void NodableLanguage::add_type(type _type, std::string _string)
{
    m_type_to_string[_type.hash_code()] = _string;
}

void NodableLanguage::add_type(type _type, Token_t _token_t, std::string _string)
{
    m_token_to_type.insert({_token_t, _type});
    m_type_to_token.insert({_type.hash_code(), _token_t});
    add_string(_string, _token_t);
    add_type(_type, _string);
}

void NodableLanguage::add_string(std::string _string, Token_t _token_t)
{
    m_token_to_string.insert({_token_t, _string});
    add_regex(std::regex("^(" + _string + ")"), _token_t);
}

void NodableLanguage::add_char(const char _char, Token_t _token_t)
{
    m_token_to_char.insert({_token_t, _char});
    m_char_to_token.insert({_char, _token_t});
}


const Signature* NodableLanguage::new_operator_signature(
        type _type,
        const Operator* _op,
        type _ltype,
        type _rtype
) const
{
    if(!_op)
    {
        return nullptr;
    }

    auto signature = new Signature( sanitize_operator_id(_op->identifier), _op);
    signature->set_return_type(_type);
    signature->push_args(_ltype, _rtype);

    NODABLE_ASSERT(signature->is_operator())
    NODABLE_ASSERT(signature->get_arg_count() == 2)

    return signature;
}

const Signature* NodableLanguage::new_operator_signature(
        type _type,
        const Operator* _op,
        type _ltype) const
{
    if(!_op)
    {
        return nullptr;
    }

    auto signature = new Signature( sanitize_operator_id(_op->identifier), _op);
    signature->set_return_type(_type);
    signature->push_arg(_ltype);

    NODABLE_ASSERT(signature->is_operator())
    NODABLE_ASSERT(signature->get_arg_count() == 1)

    return signature;
}

std::string& NodableLanguage::to_string(std::string& _out, type _type) const
{
    auto found = m_type_to_string.find(_type.hash_code());
    if( found != m_type_to_string.cend() )
    {
        return _out.append( found->second );
    }
    return _out;
}

std::string& NodableLanguage::to_string(std::string& _out, Token_t _token_t) const
{
    {
        auto found = m_token_to_char.find(_token_t);
        if (found != m_token_to_char.cend())
        {
            _out.push_back( found->second );
            return _out;
        }
    }
    auto found = m_token_to_string.find(_token_t);
    if (found != m_token_to_string.cend())
    {
        return _out.append( found->second );
    }
    return _out;
}

std::string NodableLanguage::to_string(type _type) const
{
    std::string result;
    return to_string(result, _type);
}

std::string NodableLanguage::to_string(Token_t _token) const
{
    std::string result;
    return to_string(result, _token);
}
