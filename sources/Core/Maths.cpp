#include "Maths.h"
#include <algorithm>

float Nodable::Maths::linear_interpolation(float _source, float _target, float _factor)
{
    return _source + (_target - _source ) * std::clamp(_factor, 0.0f, 1.0f);
}


