#include <nodable/core/languages/Nodable.h>

#include <nodable/core/Member.h>
#include <nodable/core/String.h>
#include <nodable/core/System.h>

#include <nodable/core/languages/Nodable_API.h> // <----- contains all functions referenced below

using namespace Nodable;
using namespace Nodable::R;
using string = std::string;

LanguageNodable::LanguageNodable()
    :
    Language("Nodable", new Parser(this), new Serializer(this))
{
    /*
     *  Configure the Semantic.
     *
     *  The order of insertion is important. First inserted will be taken in priority by Parser.
     */

    // ignored
    m_semantic.insert(std::regex("^(//(.+?)$)")      , Token_t::ignore); // Single line
    m_semantic.insert(std::regex("^(/\\*(.+?)\\*/)") , Token_t::ignore); // Multi line
    m_semantic.insert("\t", Token_t::ignore);
    m_semantic.insert(" ",  Token_t::ignore);

    // keywords
    m_semantic.insert("if"     , Token_t::keyword_if);                      // conditional structures
    m_semantic.insert("else"   , Token_t::keyword_else);
    m_semantic.insert("for"    , Token_t::keyword_for);
    m_semantic.insert("bool"   , Token_t::keyword_bool    , Type::bool_t); // types
    m_semantic.insert("string" , Token_t::keyword_string  , Type::string_t);
    m_semantic.insert("double" , Token_t::keyword_double  , Type::double_t);
    m_semantic.insert("int"    , Token_t::keyword_int     , Type::i16_t);

    // punctuation
    m_semantic.insert("{", Token_t::begin_scope);
    m_semantic.insert("}", Token_t::end_scope);
    m_semantic.insert("(", Token_t::open_bracket);
    m_semantic.insert(")", Token_t::close_bracket);
    m_semantic.insert(",", Token_t::separator);
    m_semantic.insert(";", Token_t::end_of_instruction);
    m_semantic.insert(std::string{System::k_end_of_line}, Token_t::end_of_line);

    // literals
    m_semantic.insert(std::regex("^(true|false)")                , Token_t::literal, Type::bool_t);
    m_semantic.insert(std::regex(R"(^("[^"]*"))")                , Token_t::literal, Type::string_t);
    m_semantic.insert(std::regex("^(0|([1-9][0-9]*))(\\.[0-9]+)"), Token_t::literal, Type::double_t);
    m_semantic.insert(std::regex("^(0|([1-9][0-9]*))")           , Token_t::literal, Type::i16_t);

    // identifier
    m_semantic.insert(std::regex("^([a-zA-Z_]+[a-zA-Z0-9]*)")    , Token_t::identifier);

    // operators
    m_semantic.insert(k_keyword_operator                             , Token_t::keyword_operator); // KEYWORD !
    m_semantic.insert(std::regex("^(<=>)")                           , Token_t::operator_); // 3 chars
    m_semantic.insert(std::regex("^([=\\|&]{2}|(<=)|(>=)|(=>)|(!=))"), Token_t::operator_); // 2 chars
    m_semantic.insert(std::regex("^[/+\\-*!=<>]")                    , Token_t::operator_); // single char

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
    BIND_OPERATOR_T(api_add, "+", double(double, i16_t))
    BIND_OPERATOR_T(api_add, "+", double(double, double))
    BIND_OPERATOR_T(api_add, "+", i16_t(i16_t, i16_t))
    BIND_OPERATOR_T(api_add, "+", i16_t(i16_t, double))

    BIND_OPERATOR_T(api_concat, "+", string(string, string))

    BIND_OPERATOR(api_or, "||")
    BIND_OPERATOR(api_and, "&&")

    BIND_OPERATOR_T(api_invert_sign, "-", double(double))
    BIND_OPERATOR_T(api_invert_sign, "-", i16_t(i16_t))

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
    BIND_FUNCTION_T(api_add, i16_t(i16_t, i16_t))
    BIND_FUNCTION_T(api_minus, i16_t(i16_t, i16_t))
    BIND_FUNCTION_T(api_multiply, i16_t(i16_t, i16_t))
    BIND_FUNCTION_T(api_add, double(double, double))
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
