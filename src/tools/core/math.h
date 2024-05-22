#pragma once
#include <glm/common.hpp>

namespace tools
{
    static float normalize(float _value, float _min, float _max)
    { return glm::clamp(_value, _min, _max) / (_max - _min); }

    /**
     * Interpolate linearly _source to _target with a _factor (in [0.0f, 1.0f] )
     */
    inline static float lerp(float _source, float _target, float _factor)
    { return glm::mix(_source, _target, glm::clamp(_factor, 0.0f, 1.0f)); }

    static i64_t signed_diff(u64_t _left, u64_t _right)
    {
        bool left_greater_than_right = _left > _right;
        u64_t abs_diff = left_greater_than_right ? (_left - _right) : (_right - _left);
        assert( abs_diff <= std::numeric_limits<u64_t>::max() );
        return left_greater_than_right ? (i64_t)abs_diff : -(i64_t)abs_diff;
    }
}
