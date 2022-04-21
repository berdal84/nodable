
#include <nodable/core/languages/NodableLibrary_math.h>

#include <nodable/core/String.h>
#include <nodable/core/types.h>
#include <string>
#include <cmath>
#include <nodable/core/reflection/registration.h>

using namespace Nodable;
using string = std::string;

namespace // anonymous, accessible only in that file
{
    bool _and(bool a, bool b) { return a && b; }
    bool _implies(bool a, bool b) { return !a || b; }
    bool _not(bool b) { return !b; }
    bool _or(bool a, bool b) { return a || b; }
    bool _to_bool(double n) { return n == 0.0; }
    bool _xor(bool a, bool b){ return a ^ b; }
    double _cos(double n) { return cos(n); }
    double _mod(double a, double b) { return fmod(a, b); }
    double _secondDegreePolynomial(double a, double x, double b, double y, double c) { return a * x * x + b * y + c;}
    double _sin(double n) { return sin(n); }
    std::string _to_string(bool b) { return b ? "true" : "false"; }
    std::string _to_string(double n) { return String::fmt_double(n); }
    std::string _to_string(i16_t i) { return std::to_string(i); }
    std::string _to_string(std::string s) { return s; }
    template<typename T, typename U>
    T _minus(T a, U b){ return a - b; }
    template<typename T, typename U>
    T _multiply(T a, U b) { return a * b; }
    template<typename T, typename U>
    T _plus(T a, U b){ return a + T(b); }
    template<typename T>
    T _plus(T a, T b){ return a + b; }
    template<>
    std::string _plus(std::string left, double right) { return left + Nodable::String::fmt_double(right); }
    template<>
    std::string _plus(std::string left, i16_t right) { return left + std::to_string(right); }
    template<typename T, typename U>
    T _divide(T a, U b) { if ( b == 0 ) throw std::runtime_error("division by zero !"); return a / b;}
    template<typename T, typename U>
    T _assign(T& a, U b) { return a = b; }
    template<typename T, typename U>
    T _sqrt(U n) { return sqrt(n); }
    template<typename T, typename U>
    bool _greater(T a, U b) { return a > b; }
    template<typename T, typename U>
    bool _greater_or_eq(T a, U b) { return a >= b; }
    template<typename T, typename U>
    bool _lower(T a, U b) { return a < b; }
    template<typename T, typename U>
    bool _lower_or_eq(T a, U b) { return a <= b; }
    template<typename T>
    T _pow(T a, T b) { return pow(a, b); }
    template<typename T>
    T _minus(T a) { return -a; }
    template<typename T>
    T _return(T value) { return value; }
    template<typename T>
    bool _equals(T a, T b) { return a == b; }
    template<typename T>
    bool _not_equals(T a, T b) { return a != b; }
    template<typename T>
    std::string _print(T _value)
    {
        std::string result = _to_string(_value);
        printf("print: %s\n", result.c_str());
        return result;
    }
}


REGISTER
{
    registration::push_class<NodableLibrary_math>("NodableLibrary_math")
        .add_static<double(double, i16_t)> (&_plus, "+", "plus")
        .add_static<double(double, double)>(&_plus, "+", "plus")
        .add_static<i16_t(i16_t, i16_t)>   (&_plus, "+", "plus")
        .add_static<i16_t(i16_t, double)>  (&_plus, "+", "plus")
        .add_static<string(string, string)>(&_plus, "+", "plus")
        .add_static<string(string, i16_t)> (&_plus, "+", "plus")
        .add_static<string(string, double)>(&_plus, "+", "plus")
        .add_static(&_or, "||", "or")
        .add_static(&_and, "&&", "and")
        .add_static(&_not, "!", "not")
        .add_static(&_implies, "=>", "implies")
        .add_static<double(double)>(&_minus, "-")
        .add_static<i16_t(i16_t)>(&_minus, "-")
        .add_static<double(double, double)>(&_minus, "-")
        .add_static<double(double, i16_t)>(&_minus, "-")
        .add_static<i16_t(i16_t, i16_t)>(&_minus, "-")
        .add_static<i16_t(i16_t, double)>(&_minus, "-")
        .add_static<double(double, double)>(&_divide, "/")
        .add_static<double(double, i16_t)>(&_divide, "/")
        .add_static<i16_t(i16_t, i16_t)>(&_divide, "/")
        .add_static<i16_t(i16_t, double)>(&_divide, "/")
        .add_static<double(double, double)>(&_multiply, "*")
        .add_static<double(double, i16_t)>(&_multiply, "*")
        .add_static<i16_t(i16_t, i16_t)>(&_multiply, "*")
        .add_static<i16_t(i16_t, double)>(&_multiply, "*")
        .add_static<double(double, double)>(&_minus, "-")
        .add_static<double(double, i16_t)>(&_minus, "-")
        .add_static<i16_t(i16_t, i16_t)>(&_minus, "-")
        .add_static<i16_t(i16_t, double)>(&_minus, "-")
        .add_static<bool(double, double)>(&_greater_or_eq, ">=")
        .add_static<bool(double, i16_t)>(&_greater_or_eq, ">=")
        .add_static<bool(i16_t, double)>(&_greater_or_eq, ">=")
        .add_static<bool(i16_t, i16_t)>(&_greater_or_eq, ">=")
        .add_static<bool(double, i16_t)>(&_lower_or_eq, "<=")
        .add_static<bool(double, double)>(&_lower_or_eq, "<=")
        .add_static<bool(i16_t, i16_t)>(&_lower_or_eq, "<=")
        .add_static<bool(i16_t, double)>(&_lower_or_eq, "<=")
        .add_static<string(string& , string)>(&_assign, "=")
        .add_static<bool(bool& , bool)>(&_assign, "=")
        .add_static<double(double& , i16_t)>(&_assign, "=")
        .add_static<double(double& , double)>(&_assign, "=")
        .add_static<i16_t(i16_t & , i16_t)>(&_assign, "=")
        .add_static<i16_t(i16_t & , double)>(&_assign, "=")
        .add_static<bool(i16_t, i16_t)>(&_equals, "==")
        .add_static<bool(double, double)>(&_equals, "==")
        .add_static<bool(string, string)>(&_equals, "==")
        .add_static<bool(bool, bool)>(&_equals, "<=>")
        .add_static<bool(bool, bool)>(&_not_equals, "!=")
        .add_static<bool(i16_t, i16_t)>(&_not_equals, "!=")
        .add_static<bool(double, double)>(&_not_equals, "!=")
        .add_static<bool(string, string)>(&_not_equals, "!=")
        .add_static<bool(double, double)>(&_greater, ">")
        .add_static< bool(i16_t, i16_t)>(&_greater, ">")
        .add_static<bool(double, double)>(&_lower, "<")
        .add_static<bool(i16_t, i16_t)>(&_lower, "<")
        .add_static<bool(bool)>(&_return, "return")
        .add_static<i16_t(i16_t)>(&_return, "return")
        .add_static<double(double)>(&_return, "return")
        .add_static<string(string)>(&_return, "return")
        .add_static(&_sin, "sin")
        .add_static(&_cos, "cos")
        .add_static<double(double)>(&_sqrt, "sqrt")
        .add_static<i16_t(i16_t)>(&_sqrt, "sqrt")
        .add_static(&_to_bool, "to_bool")
        .add_static(&_mod, "mod")
        .add_static<i16_t(i16_t, i16_t)>(&_pow, "pow")
        .add_static<double(double, double)>(&_pow, "pow")
        .add_static(&_secondDegreePolynomial, "secondDegreePolynomial" )
        .add_static<string(bool)>(&_to_string, "to_string")
        .add_static<string(double)>(&_to_string, "to_string")
        .add_static<string(i16_t)>(&_to_string, "to_string")
        .add_static<string(string)>(&_to_string, "to_string")
        .add_static<string(bool)>(&_print, "print")
        .add_static<string(double)>(&_print, "print")
        .add_static<string(i16_t)>(&_print, "print")
        .add_static<string(string)>(&_print, "print");
};

NodableLibrary_math::NodableLibrary_math(){}