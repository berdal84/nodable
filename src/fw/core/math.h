#pragma once
#include <imgui/imgui.h>
#include "types.h"

namespace fw::math
{
    static float clamp(float _value, float _min, float _max)
    {
        if ( _value < _min ) return _min;
        if ( _value > _max ) return _max;
        return _value;
    }

    static float normalize(float _value, float _min, float _max)
    {
        return clamp(_value, _min, _max) / (_max - _min);
    }

    /**
     * Interpolate linearly _source to _target with a _factor (in [0.0f, 1.0f] )
     */
    static float lerp(float _source, float _target, float _factor)
    {
        return _source + (_target - _source ) * clamp(_factor, 0.0f, 1.0f);
    }

    static i64_t signed_diff(u64_t _left, u64_t _right)
    {
        bool left_greater_than_right = _left > _right;
        u64_t abs_diff = left_greater_than_right ? (_left - _right) : (_right - _left);
        FW_ASSERT( abs_diff <= std::numeric_limits<u64_t>::max() );
        return left_greater_than_right ? (i64_t)abs_diff : -(i64_t)abs_diff;
    }

    static ImVec2 round(ImVec2 vec)
    {
        return {(float)(int)(vec.x), (float)(int)(vec.y)};
    }
}
