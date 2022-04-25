#pragma once
#include <nodable/core/types.h>

namespace ndbl::math
{
    /**
     * Interpolate linearly _source to _target with a _factor (in [0.0f, 1.0f] )
     * @param _source
     * @param _target
     * @param _factor
     * @return
     */
    static float lerp(float _source, float _target, float _factor)
    {
        return _source + (_target - _source ) * std::clamp(_factor, 0.0f, 1.0f);
    }

    static i64_t signed_diff(u64_t _left, u64_t _right)
    {
        bool left_greater_than_right = _left > _right;
        u64_t abs_diff = left_greater_than_right ? (_left - _right) : (_right - _left);
        NODABLE_ASSERT( abs_diff <= std::numeric_limits<u64_t>::max() );
        return left_greater_than_right ? (i64_t)abs_diff : -(i64_t)abs_diff;
    }
}
