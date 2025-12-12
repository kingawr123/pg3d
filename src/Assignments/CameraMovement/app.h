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
#include "camera_controller.h"
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
        camera()->perspective(fov, aspect, near, far);
        camera()->look_at(glm::vec3(-2.0f, 1.0f, 3.0f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f));

        glGenBuffers(1, &u_pvm_buffer_);
    }

    void init() override;

    void frame() override;

    void framebuffer_resize_callback(int w, int h) override;

    void set_camera(Camera *camera) { camera_ = camera; }

    Camera *camera() { return camera_; }

    void scroll_callback(double xoffset, double yoffset) override {
        Application::scroll_callback(xoffset, yoffset);
        camera()->zoom(yoffset / 5.0f);
    }

    void set_controller(CameraController *controller) { controller_ = controller; }

    void mouse_button_callback(int button, int action, int mods) {
        Application::mouse_button_callback(button, action, mods);

        if (controller_) {
            double x, y;
            glfwGetCursorPos(window_, &x, &y);

            if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
                controller_->LMB_pressed(x, y);

            if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
                controller_->LMB_released(x, y);
        }

    }

    void cursor_position_callback(double x, double y) {
        Application::cursor_position_callback(x, y);
        if (controller_) {
            controller_->mouse_moved(x, y);
        }
    }

    ~SimpleShapeApplication() {
        if (camera_) {
            delete camera_;
        }
    }
private:
    GLuint vao_;
    Camera *camera_;
    CameraController *controller_;
    GLuint u_pvm_buffer_;
};
