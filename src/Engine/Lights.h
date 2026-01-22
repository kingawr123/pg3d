#pragma once

#include "glm/glm.hpp"

namespace xe {
    struct PointLight {
        PointLight() = default;
        PointLight(const glm::vec3 &pos, const glm::vec3 &color, float intensity, float radius)
                : position_in_ws(pos),
                  color(color), intensity(intensity), radius(radius) {}

        glm::vec3 position_in_ws;
        glm::vec3 position_in_vs;
        glm::vec3 color;
        float intensity;
        float radius;
    };
}