//
// Created by pbialas on 05.08.2020.
//

#pragma once

#include <vector>

#include "Application/application.h"
#include "Application/utils.h"

#include "glad/gl.h"
#include <glm/glm.hpp>

#include "camera.h"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/scalar_constants.hpp"


class SimpleShapeApplication : public xe::Application
{
public:
    SimpleShapeApplication(int width, int height, std::string title, bool debug) : Application(width, height, title, debug) {
        auto [w, h] = frame_buffer_size();
        float aspect = static_cast<float>(w)/h;
        float fov = glm::pi<float>()/4.0;
        float near = 0.1f;
        float far = 100.0f;

        set_camera(new Camera);
        camera_->perspective(fov, aspect, near, far);
        camera_->look_at(glm::vec3(3.0f, 0.0f, 2.0f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f));

        glGenBuffers(1, &u_pvm_buffer_);
        glGenBuffers(1, &u_pvm_buffer_);
    }

    void init() override;

    void frame() override;

    void framebuffer_resize_callback(int w, int h) override;

    void set_camera(Camera *camera) { camera_ = camera; }

    Camera *camera() { return camera_; }

    void scroll_callback(double xoffset, double yoffset) override {
        Application::scroll_callback(xoffset, yoffset);
        camera()->zoom(yoffset / 30.0f);
    }

    ~SimpleShapeApplication() {
        if (camera_) {
            delete camera_;
        }
    }
private:
    GLuint vao_;
    Camera *camera_;
    GLuint u_pvm_buffer_;
};
