#include "Nodlang_math.h"

#include <string>
#include <cmath>

#include "fw/core/format.h"
#include "fw/core/reflection/registration.h"
#include "fw/core/types.h"

using namespace ndbl;

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
    std::string _to_string(double n) { return fw::format::number(n); }
    std::string _to_string(i32_t i) { return std::to_string(i); }
    std::string _to_string(std::string s) { return s; }
    template<typename T, typename U>
    T _minus(T a, U b){ return a - T(b); }
    template<typename T, typename U>
    T _multiply(T a, U b) { return a * T(b); }
    template<typename T, typename U>
    T _plus(T a, U b){ return a + T(b); }
    template<typename T>
    T _plus(T a, T b){ return a + T(b); }
    template<>
    std::string _plus(std::string left, double right) { return left + fw::format::number(right); }
    template<>
    std::string _plus(std::string left, i32_t right) { return left + std::to_string(right); }
    template<typename T, typename U>
    T _divide(T a, U b) { if ( b == 0 ) throw std::runtime_error("division by zero !"); return a / T(b);}
    template<typename T, typename U>
    T _assign(T& a, U b) { return a = T(b); }
    template<typename T>
    T _sqrt(T n) { return (T)sqrt((float)n); }
    template<typename T, typename U>
    bool _greater(T a, U b) { return a > b; }
    template<typename T, typename U>
    bool _greater_or_eq(T a, U b) { return a >= b; }
    template<typename T, typename U>
    bool _lower(T a, U b) { return a < b; }
    template<typename T, typename U>
    bool _lower_or_eq(T a, U b) { return a <= b; }
    template<typename T>
    T _pow(T a, T b) { return (T)pow(a, b); }
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
    fw::registration::push_class<Nodlang_math>("Nodlang_math")
        .add_method<double(double, i32_t)> (&_plus, "+", "plus")
        .add_method<double(double, double)>(&_plus, "+", "plus")
        .add_method<i32_t(i32_t, i32_t)>   (&_plus, "+", "plus")
        .add_method<i32_t(i32_t, double)>  (&_plus, "+", "plus")
        .add_method<std::string(std::string, std::string)>(&_plus, "+", "plus")
        .add_method<std::string(std::string, i32_t)> (&_plus, "+", "plus")
        .add_method<std::string(std::string, double)>(&_plus, "+", "plus")
        .add_method(&_or, "||", "or")
        .add_method(&_and, "&&", "and")
        .add_method(&_not, "!", "not")
        .add_method(&_implies, "=>", "implies")
        .add_method<double(double)>(&_minus, "-")
        .add_method<i32_t(i32_t)>(&_minus, "-")
        .add_method<double(double, double)>(&_minus, "-")
        .add_method<double(double, i32_t)>(&_minus, "-")
        .add_method<i32_t(i32_t, i32_t)>(&_minus, "-")
        .add_method<i32_t(i32_t, double)>(&_minus, "-")
        .add_method<double(double, double)>(&_divide, "/")
        .add_method<double(double, i32_t)>(&_divide, "/")
        .add_method<i32_t(i32_t, i32_t)>(&_divide, "/")
        .add_method<i32_t(i32_t, double)>(&_divide, "/")
        .add_method<double(double, double)>(&_multiply, "*")
        .add_method<double(double, i32_t)>(&_multiply, "*")
        .add_method<i32_t(i32_t, i32_t)>(&_multiply, "*")
        .add_method<i32_t(i32_t, double)>(&_multiply, "*")
        .add_method<double(double, double)>(&_minus, "-")
        .add_method<double(double, i32_t)>(&_minus, "-")
        .add_method<i32_t(i32_t, i32_t)>(&_minus, "-")
        .add_method<i32_t(i32_t, double)>(&_minus, "-")
        .add_method<bool(double, double)>(&_greater_or_eq, ">=")
        .add_method<bool(double, i32_t)>(&_greater_or_eq, ">=")
        .add_method<bool(i32_t, double)>(&_greater_or_eq, ">=")
        .add_method<bool(i32_t, i32_t)>(&_greater_or_eq, ">=")
        .add_method<bool(double, i32_t)>(&_lower_or_eq, "<=")
        .add_method<bool(double, double)>(&_lower_or_eq, "<=")
        .add_method<bool(i32_t, i32_t)>(&_lower_or_eq, "<=")
        .add_method<bool(i32_t, double)>(&_lower_or_eq, "<=")
        .add_method<std::string(std::string& , std::string)>(&_assign, "=")
        .add_method<bool(bool& , bool)>(&_assign, "=")
        .add_method<double(double& , i32_t)>(&_assign, "=")
        .add_method<double(double& , double)>(&_assign, "=")
        .add_method<i32_t(i32_t & , i32_t)>(&_assign, "=")
        .add_method<i32_t(i32_t & , double)>(&_assign, "=")
        .add_method<bool(i32_t, i32_t)>(&_equals, "==")
        .add_method<bool(double, double)>(&_equals, "==")
        .add_method<bool(std::string, std::string)>(&_equals, "==")
        .add_method<bool(bool, bool)>(&_equals, "<=>")
        .add_method<bool(bool, bool)>(&_not_equals, "!=")
        .add_method<bool(i32_t, i32_t)>(&_not_equals, "!=")
        .add_method<bool(double, double)>(&_not_equals, "!=")
        .add_method<bool(std::string, std::string)>(&_not_equals, "!=")
        .add_method<bool(double, double)>(&_greater, ">")
        .add_method< bool(i32_t, i32_t)>(&_greater, ">")
        .add_method<bool(double, double)>(&_lower, "<")
        .add_method<bool(i32_t, i32_t)>(&_lower, "<")
        .add_method<bool(bool)>(&_return, "return")
        .add_method<i32_t(i32_t)>(&_return, "return")
        .add_method<double(double)>(&_return, "return")
        .add_method<std::string(std::string)>(&_return, "return")
        .add_method(&_sin, "sin")
        .add_method(&_cos, "cos")
        .add_method<double(double)>(&_sqrt, "sqrt")
        .add_method<i32_t(i32_t)>(&_sqrt, "sqrt")
        .add_method(&_to_bool, "to_bool")
        .add_method(&_mod, "mod")
        .add_method<i32_t(i32_t, i32_t)>(&_pow, "pow")
        .add_method<double(double, double)>(&_pow, "pow")
        .add_method(&_secondDegreePolynomial, "secondDegreePolynomial" )
        .add_method<std::string(bool)>(&_to_string, "to_string")
        .add_method<std::string(double)>(&_to_string, "to_string")
        .add_method<std::string(i32_t)>(&_to_string, "to_string")
        .add_method<std::string(std::string)>(&_to_string, "to_string")
        .add_method<std::string(bool)>(&_print, "print")
        .add_method<std::string(double)>(&_print, "print")
        .add_method<std::string(i32_t)>(&_print, "print")
        .add_method<std::string(std::string)>(&_print, "print");
};

Nodlang_math::Nodlang_math(){} // necessary to trigger static code execution