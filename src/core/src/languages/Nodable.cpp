#include <nodable/core/languages/Nodable.h>

#include <nodable/core/Member.h>
#include <nodable/core/String.h>
#include <nodable/core/System.h>

// Nodable API begin
#include <nodable/core/languages/Nodable_math.h>
#include <nodable/core/languages/Nodable_biology.h>
// Nodable API end

using namespace Nodable;
using namespace Nodable::math;
using namespace Nodable::biology;

using string = std::string;

LanguageNodable::LanguageNodable(): Language("Nodable")
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

std::string LanguageNodable::sanitize_function_id(const std::string &_id) const
{
    if( _id.compare(0, 4, "api_") == 0)
    {
        return _id.substr(4);
    }
    return _id;
}

std::string LanguageNodable::sanitize_operator_id(const std::string &_id) const
{
    return k_keyword_operator + _id;
}
