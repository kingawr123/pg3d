//
// Created by kinga on 12/12/2025.
//

#ifndef GRAPHICS3DCODE_CAMERA_CONTROLLER_H
#define GRAPHICS3DCODE_CAMERA_CONTROLLER_H

#include "camera.h"

class CameraController {
public:
    CameraController():camera_(nullptr) {
        LMB_pressed_ = false;
    }
    explicit CameraController(Camera* camera):camera_(camera) {
        LMB_pressed_ = false;
    }
    void set_camera(Camera *camera) { camera_ = camera; }

    void rotate_camera(float dx, float dy) {
        camera_->rotate_around_center(-scale_ * dy, camera_->x());
        camera_->rotate_around_center(-scale_ * dx, camera_->y());
    }

    void mouse_moved(float x, float y) {
        if (LMB_pressed_) {
            auto dx = x - x_;
            auto dy = y - y_;
            x_ = x;
            y_ = y;

            rotate_camera(dx, dy);
        }
    };

    void LMB_pressed(float x, float y) {
        LMB_pressed_ = true;
        x_ = x;
        y_ = y;
    };

    void LMB_released(float x, float y) {
        LMB_pressed_ = false;
        auto dx = x - x_;
        auto dy = y - y_;

        rotate_camera(dx, dy);
    };

private:
    Camera *camera_;
    float scale_ = 0.1f;
    bool LMB_pressed_ = false;
    float x_ = 0.0f, y_ = 0.0f;
};

#endif //GRAPHICS3DCODE_CAMERA_CONTROLLER_H