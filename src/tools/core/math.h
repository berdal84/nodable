#pragma once
#include <glm/common.hpp>

namespace tools
{
    static float normalize(float _value, float _min, float _max)
    { return glm::clamp(_value, _min, _max) / (_max - _min); }

    template<typename T>
    inline static float lerp(T _source, T _target, T _factor);

    inline float wave(float min, float max, double time, float speed)
    {
        float factor = 0.5f * ( 1.0f + std::sin( time * speed ) );
        return lerp(min, max, factor);
    }

    static i64_t signed_diff(u64_t a, u64_t b)
    {
        if ( a == b )
            return 0;

        i64_t sign =  a > b ? 1 : -1;
        u64_t unsigned_diff = sign > 0 ? a - b : b - a;

        ASSERT(unsigned_diff <= std::numeric_limits<i64_t>::max() ); // capacity check
        ASSERT(unsigned_diff <= std::numeric_limits<i64_t>::min() ); // capacity check

        return sign * (i64_t)unsigned_diff;
    }

    template<typename T>
    inline static float lerp(T _source, T _target, T _factor)
    {
        static_assert( std::is_arithmetic_v<T> );
        return glm::mix(_source, _target, glm::clamp(_factor, T{0}, T{1}));
    }

}
