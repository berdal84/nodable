
#include <nodable/core/languages/NodableLibrary_math.h>
#include <nodable/core/Language_MACROS.h>
#include <nodable/core/String.h>
#include <nodable/core/types.h>
#include <string>
#include <cmath>

using namespace Nodable;
using string = std::string;

bool api_and(bool a, bool b) { return a && b; }
bool api_implies(bool a, bool b) { return !a || b; }
bool api_not(bool b) { return !b; }
bool api_or(bool a, bool b) { return a || b; }
bool api_to_bool(double n) { return n == 0.0; }
bool api_xor(bool a, bool b){ return a ^ b; }
double api_cos(double n) { return cos(n); }
double api_mod(double a, double b) { return fmod(a, b); }
double api_secondDegreePolynomial(double a, double x, double b, double y, double c) { return a * x * x + b * y + c;}
double api_sin(double n) { return sin(n); }
std::string api_to_string(bool b) { return b ? "true" : "false"; }
std::string api_to_string(double n) { return String::fmt_double(n); }
std::string api_to_string(i16_t i) { return std::to_string(i); }
std::string api_to_string(std::string s) { return s; }
template<typename T, typename U>
T api_minus(T a, U b){ return a - b; }
template<typename T, typename U>
T api_multiply(T a, U b) { return a * b; }
template<typename T, typename U>
T api_plus(T a, U b){ return a + T(b); }
template<typename T>
T api_plus(T a, T b){ return a + b; }
template<>
std::string api_plus(std::string left, double right) { return left + Nodable::String::fmt_double(right); }
template<>
std::string api_plus(std::string left, i16_t right) { return left + std::to_string(right); }
template<typename T, typename U>
T api_divide(T a, U b) { if ( b == 0 ) throw std::runtime_error("division by zero !"); return a / b;}
template<typename T, typename U>
T api_assign(T& a, U b) { return a = b; }
template<typename T, typename U>
T api_sqrt(U n) { return sqrt(n); }
template<typename T, typename U>
bool api_greater(T a, U b) { return a > b; }
template<typename T, typename U>
bool api_greater_or_eq(T a, U b) { return a >= b; }
template<typename T, typename U>
bool api_lower(T a, U b) { return a < b; }
template<typename T, typename U>
bool api_lower_or_eq(T a, U b) { return a <= b; }
template<typename T>
T api_pow(T a, T b) { return pow(a, b); }
template<typename T>
T api_minus(T a) { return -a; }
template<typename T>
T api_return(T value) { return value; }
template<typename T>
bool api_equals(T a, T b) { return a == b; }
template<typename T>
bool api_not_equals(T a, T b) { return a != b; }
template<typename T>
std::string api_print(T _value)
{
    std::string result = api_to_string(_value);
    printf("print: %s\n", result.c_str());
    return result;
}

void NodableLibrary_math::bind_to_language(ILanguage* _language)const
{
    BIND_TO(_language)

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
    BIND_FUNCTION_T(api_to_string, string(bool))
    BIND_FUNCTION_T(api_to_string, string(double))
    BIND_FUNCTION_T(api_to_string, string(i16_t))
    BIND_FUNCTION_T(api_to_string, string(string))
    BIND_FUNCTION_T(api_print, string(bool))
    BIND_FUNCTION_T(api_print, string(double))
    BIND_FUNCTION_T(api_print, string(i16_t))
    BIND_FUNCTION_T(api_print, string(string))
}
