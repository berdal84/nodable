#pragma once

namespace Nodable::Maths
{
    /**
     * Interpolate linearly _source to _target with a _factor (in [0.0f, 1.0f] )
     * @param _source
     * @param _target
     * @param _factor
     * @return
     */
    float lerp(float _source, float _target, float _factor)
    {
        return _source + (_target - _source ) * std::clamp(_factor, 0.0f, 1.0f);
    }
}
