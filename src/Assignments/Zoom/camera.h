//
// Created by kinga on 06/12/2025.
//

#ifndef GRAPHICS3DCODE_CAMERA_H
#define GRAPHICS3DCODE_CAMERA_H

#pragma once
#include <cmath>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

class Camera {
public:
    void look_at(const glm::vec3 &eye, const glm::vec3 &center, const glm::vec3 &up) {
        V_ = glm::lookAt(eye, center, up);
    }

    void perspective(float fov, float aspect, float near, float far) {
        fov_ = fov;
        aspect_ = aspect;
        near_ = near;
        far_ = far;
    }

    void set_aspect(float aspect) {
        aspect_ = aspect;
    }

    glm::mat4 view() const { return V_; }

    glm::mat4 projection() const { return glm::perspective(fov_, aspect_, near_, far_); }

    void zoom(float y_offset) {
        auto y = inverse_logistics(fov_ / glm::pi<float>());
        y += y_offset;
        auto fov_new = logistic(y) * glm::pi<float>();

        const float FOV_MIN = glm::radians(5.0f);
        const float FOV_MAX = glm::radians(90.0f);
        fov_ = std::clamp(fov_new, FOV_MIN, FOV_MAX);
    }

private:
    float fov_;
    float aspect_;
    float near_;
    float far_;

    glm::mat4 V_;

    static float logistic(float y) {
        return 1.0f/(1.0f+std::exp(-y));
    }

    static float inverse_logistics(float x) {
        return std::log(x/(1.0f-x));
    }
};

#endif //GRAPHICS3DCODE_CAMERA_H