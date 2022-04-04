#include <nodable/core/languages/Nodable.h>

#include <nodable/core/Member.h>
#include <nodable/core/Format.h>
#include <nodable/core/System.h>

#include <nodable/core/languages/Nodable_API.h> // <----- contains all functions referenced below

using namespace Nodable;
using namespace Nodable::R;

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
    m_semantic.insert(std::regex("^(//(.+?)$)"), TokenType_Ignore);      // Single line
    m_semantic.insert(std::regex("^(/\\*(.+?)\\*/)"), TokenType_Ignore); // Multi line
    m_semantic.insert("\t", TokenType_Ignore);
    m_semantic.insert(" ", TokenType_Ignore);

    // keywords
    m_semantic.insert("if", TokenType_KeywordIf);                      // conditional structures
    m_semantic.insert("else", TokenType_KeywordElse);
    m_semantic.insert("for", TokenType_KeywordFor);
    m_semantic.insert("bool", TokenType_KeywordBoolean, Type::Boolean); // types
    m_semantic.insert("string", TokenType_KeywordString, Type::String);
    m_semantic.insert("double", TokenType_KeywordDouble, Type::Double);
    m_semantic.insert("int", TokenType_KeywordInt, Type::Int16);

    // punctuation
    m_semantic.insert("{", TokenType_BeginScope);
    m_semantic.insert("}", TokenType_EndScope);
    m_semantic.insert("(", TokenType_OpenBracket);
    m_semantic.insert(")", TokenType_CloseBracket);
    m_semantic.insert(",", TokenType_Separator);
    m_semantic.insert(";", TokenType_EndOfInstruction);
    m_semantic.insert(std::string{System::k_end_of_line}, TokenType_EndOfLine);

    // literals
    m_semantic.insert(std::regex("^(true|false)"), TokenType_Literal, Type::Boolean);
    m_semantic.insert(std::regex(R"(^("[^"]*"))"), TokenType_Literal, Type::String);
    m_semantic.insert(std::regex("^(0|([1-9][0-9]*))(\\.[0-9]+)?"), TokenType_Literal, Type::Double);
    m_semantic.insert(std::regex("^(0|([1-9][0-9]*))"), TokenType_Literal, Type::Int16);

    // identifier
    m_semantic.insert(std::regex("^([a-zA-Z_]+[a-zA-Z0-9]*)"), TokenType_Identifier);

    // operators
    m_semantic.insert("operator", TokenType_KeywordOperator); // 3 chars
    m_semantic.insert(std::regex("^(<=>)"), TokenType_Operator); // 3 chars
    m_semantic.insert(std::regex("^([=\\|&]{2}|(<=)|(>=)|(=>)|(!=))"), TokenType_Operator); // 2 chars
    m_semantic.insert(std::regex("^[/+\\-*!=<>]"), TokenType_Operator); // single char

    add( new Operator("-"  , Operator_t::Unary, 5));
    add( new Operator("!"  , Operator_t::Unary, 5));

    add( new Operator("="  , Operator_t::Binary, 0));
    add( new Operator("+"  , Operator_t::Binary, 10));
    add( new Operator("-"  , Operator_t::Binary, 10));
    add( new Operator("||" , Operator_t::Binary, 10));
    add( new Operator("&&" , Operator_t::Binary, 10));
    add( new Operator("/"  , Operator_t::Binary, 10));
    add( new Operator("*"  , Operator_t::Binary, 10));
    add( new Operator(">=" , Operator_t::Binary, 10));
    add( new Operator("<=" , Operator_t::Binary, 10));
    add( new Operator("=>" , Operator_t::Binary, 10));
    add( new Operator("==" , Operator_t::Binary, 10));
    add( new Operator("<=>", Operator_t::Binary, 10));
    add( new Operator("!=" , Operator_t::Binary, 10));
    add( new Operator(">"  , Operator_t::Binary, 10));
    add( new Operator("<"  , Operator_t::Binary, 10));

    /*
     * Wrap a minimal set of functions/operators
     */
    using string = std::string;

    WRAP_POLYFUNC(api_return, bool(bool))
    WRAP_POLYFUNC(api_return, double(double))
    WRAP_POLYFUNC(api_return, string(string))
    WRAP_FUNCTION(api_sin)
    WRAP_FUNCTION(api_cos)
    WRAP_POLYFUNC(api_add,      i16_t(i16_t, i16_t))
    WRAP_POLYFUNC(api_minus,    i16_t(i16_t, i16_t))
    WRAP_POLYFUNC(api_multiply, i16_t(i16_t, i16_t))
    WRAP_POLYFUNC(api_add,      double(double, double))
    WRAP_POLYFUNC(api_minus,    double(double, double))
    WRAP_POLYFUNC(api_multiply, double(double, double))
    WRAP_FUNCTION(api_sqrt)
    WRAP_FUNCTION(api_not)
    WRAP_FUNCTION(api_or)
    WRAP_FUNCTION(api_and)
    WRAP_FUNCTION(api_xor)
    WRAP_FUNCTION(api_to_bool)
    WRAP_FUNCTION(api_mod)
    WRAP_POLYFUNC(api_pow, i16_t(i16_t, i16_t))
    WRAP_POLYFUNC(api_pow, double(double, double))
    WRAP_FUNCTION(api_secondDegreePolynomial)
	WRAP_FUNCTION(api_DNAtoProtein)

    WRAP_POLYFUNC(api_to_string, string(bool))
    WRAP_POLYFUNC(api_to_string, string(double))
    WRAP_POLYFUNC(api_to_string, string(i16_t))
    WRAP_POLYFUNC(api_to_string, string(string))

    WRAP_POLYFUNC(api_print,     string(bool))
    WRAP_POLYFUNC(api_print,     string(double))
    WRAP_POLYFUNC(api_print,     string(i16_t))
    WRAP_POLYFUNC(api_print, string(string))

    WRAP_POLYOPER(api_add        , "+" , ICON_FA_PLUS " Add", double(double, double))
    WRAP_POLYOPER(api_add        , "+" , ICON_FA_PLUS " Add", i16_t(i16_t, i16_t))

    WRAP_POLYOPER(api_concat     , "+" , "Concat.", string(string, string))
    WRAP_POLYOPER(api_concat     , "+" , "Concat.", string(string, double))
    WRAP_POLYOPER(api_concat     , "+" , "Concat.", string(string, i16_t))
    WRAP_POLYOPER(api_concat     , "+" , "Concat.", string(string, bool))

    WRAP_OPERATOR(api_or         , "||", "Logical Or")
    WRAP_OPERATOR(api_and        , "&&", "Logical And")

    WRAP_POLYOPER(api_invert_sign, "-" , ICON_FA_MINUS " Invert Sign", double(double))
    WRAP_POLYOPER(api_minus      , "-" , ICON_FA_MINUS " Subtract",    double(double, double))
    WRAP_POLYOPER(api_divide     , "/" , ICON_FA_DIVIDE " Divide",     double(double, double))
    WRAP_POLYOPER(api_multiply   , "*" , ICON_FA_TIMES " Multiply",    double(double, double))
    WRAP_POLYOPER(api_minus      , "-" , ICON_FA_MINUS " Minus",       double(double, double))
    WRAP_POLYOPER(api_greater_or_eq, ">=", ">= Greater or equal",      bool(double, double))
    WRAP_POLYOPER(api_lower_or_eq, "<=", "<= Less or equal",           bool(double, double))

    WRAP_POLYOPER(api_invert_sign, "-" , ICON_FA_MINUS " Invert Sign", i16_t(i16_t))
    WRAP_POLYOPER(api_minus      , "-" , ICON_FA_MINUS " Subtract",    i16_t(i16_t, i16_t))
    WRAP_POLYOPER(api_divide     , "/" , ICON_FA_DIVIDE " Divide",     i16_t(i16_t, i16_t))
    WRAP_POLYOPER(api_multiply   , "*" , ICON_FA_TIMES " Multiply",    i16_t(i16_t, i16_t))
    WRAP_POLYOPER(api_minus      , "-" , ICON_FA_MINUS " Minus",       i16_t(i16_t, i16_t))
    WRAP_POLYOPER(api_greater_or_eq, ">=", ">= Greater or equal",      bool(i16_t, i16_t))
    WRAP_POLYOPER(api_lower_or_eq, "<=", "<= Less or equal",           bool(i16_t, i16_t))
    WRAP_OPERATOR(api_not        , "!" , "! not")
    WRAP_POLYOPER(api_assign     , "=" , ICON_FA_EQUALS " Assign", string(string&, string) )
    WRAP_POLYOPER(api_assign     , "=" , ICON_FA_EQUALS " Assign", bool(bool&, bool) )
    WRAP_POLYOPER(api_assign     , "=" , ICON_FA_EQUALS " Assign", double(double&, double) )
    WRAP_POLYOPER(api_assign     , "=" , ICON_FA_EQUALS " Assign", i16_t(i16_t&, i16_t) )
    WRAP_OPERATOR(api_implies    , "=>", "=> Implies")
    WRAP_POLYOPER(api_equals  , "==", "== Equals" , bool(i16_t, i16_t) )
    WRAP_POLYOPER(api_equals  , "==", "== Equals" , bool(double, double) )
    WRAP_POLYOPER(api_equals  , "==", "== Equals" , bool(string, string) )
    WRAP_POLYOPER(api_equals  , "<=>", "<=> Equivalent" , bool(bool, bool) )
    WRAP_POLYOPER(api_not_equals  , "!=", "!= Not equal" , bool(bool, bool) )
    WRAP_POLYOPER(api_not_equals  , "!=", "!= Not equal" , bool(i16_t, i16_t) )
    WRAP_POLYOPER(api_not_equals  , "!=", "!= Not equal" , bool(double, double) )
    WRAP_POLYOPER(api_not_equals  , "!=", "!= Not equal" , bool(string, string) )
    WRAP_POLYOPER(api_greater ,">" , "> Greater", bool(double, double))
    WRAP_POLYOPER(api_greater ,">" , "> Greater", bool(i16_t, i16_t))
    WRAP_POLYOPER(api_lower   , "<", "< Less", bool(double, double))
    WRAP_POLYOPER(api_lower, "<", "< Less", bool(i16_t, i16_t))
}

void LanguageNodable::sanitize_function_identifier( std::string& _identifier ) const
{
    _identifier = regex_replace(_identifier, std::regex("^api_"), "");
}

void LanguageNodable::sanitize_operator_fct_identifier( std::string& _identifier ) const
{
    _identifier.insert(0, "operator");
}
