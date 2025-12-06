//
// Created by pbialas on 05.08.2020.
//

#pragma once

#include <vector>

#include "Application/application.h"
#include "Application/utils.h"

#include "glad/gl.h"
#include <glm/glm.hpp>

#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/scalar_constants.hpp"


class SimpleShapeApplication : public xe::Application
{
public:
    SimpleShapeApplication(int width, int height, std::string title, bool debug) : Application(width, height, title, debug) {
        auto [w, h] = frame_buffer_size();
        aspect_ = static_cast<float>(w)/h;
        fov_ = glm::pi<float>()/4.0;
        near_ = 0.1f;
        far_ = 100.0f;
        P_ = glm::perspective(fov_, aspect_, near_, far_);
        V_ = glm::lookAt(
        glm::vec3(3.0f, 0.0f, 2.0f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f));

        glGenBuffers(1, &u_pvm_buffer_);
        glGenBuffers(1, &u_pvm_buffer_);
    }

    void init() override;

    void frame() override;

    void framebuffer_resize_callback(int w, int h) override;
private:
    GLuint vao_;
    float fov_;
    float aspect_;
    float near_;
    float far_;

    glm::mat4 P_;
    glm::mat4 V_;

    GLuint u_pvm_buffer_;
};
