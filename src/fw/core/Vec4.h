#pragma once
#include "imgui/imgui.h"
#include <glm/glm/common.hpp>
#include <glm/glm/vec4.hpp>

namespace fw
{
    class Vec4
    {
    public:
        float x{};
        float y{};
        float z{};
        float w{};

        constexpr Vec4() = default;
        constexpr Vec4(float x, float y, float z, float w = 0.0f)
        : x(x), y(y), z(z), w(w)
        {}
        constexpr Vec4(const glm::vec4& vec): Vec4(vec.x, vec.y, vec.z, vec.w) {}

        template<typename VecT>
        constexpr explicit Vec4(const VecT& _vec)
        : Vec4(_vec.x, _vec.y, _vec.z, _vec.w)
        {}

        operator glm::vec4() const
        { return {x, y, z, w}; }

        operator ImVec4() const
        { return {x, y, z, w}; }

        inline static Vec4 lerp(Vec4 _source, Vec4 _target, float _factor)
        { return glm::mix((glm::vec4)_source, (glm::vec4)_target, glm::clamp(_factor, 0.0f, 1.0f)); }
    };
}
