#pragma once

#include <ctime>
#include <cmath>
#include <string>
#include <nodable/core/String.h>

namespace Nodable {
namespace lib {
namespace math {

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
} // namespace math
} // namespace lib
} // namespace Nodable