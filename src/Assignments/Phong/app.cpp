//
// Created by pbialas on 25.09.2020.
//

#include "app.h"

#include <iostream>
#include <vector>
#include <tuple>
#include <numeric>

#include <glm/glm.hpp>

#include "Application/utils.h"
#include "glm/gtc/type_ptr.hpp"
#include "Engine/Mesh.h"
#include "Engine/mesh_loader.h"

#define STB_IMAGE_IMPLEMENTATION  1

#include "3rdParty/stb/stb_image.h"
#include "Engine/ColorMaterial.h"
#include "Engine/PhongMaterial.h"


struct Std140Vec3 {
    glm::vec3 v;
    float _pad = 0.0f; // pad to 16 bytes
};


struct alignas(16) Std140PointLight {
    Std140Vec3 position_in_view_space;
    Std140Vec3 color;
    float intensity;
    float radius;
    float _pad0 = 0.0f;
    float _pad1 = 0.0f;
};

static_assert(sizeof(Std140PointLight) == 48, "Std140PointLight must be 48 bytes");

struct Std140LightsHeader {
    Std140Vec3 ambient;
    uint32_t n_p_lights;
    uint32_t _pad0 = 0;
};

void SimpleShapeApplication::init() {

    xe::ColorMaterial::init();
    xe::PhongMaterial::init();

    set_controller(new CameraController(camera()));

    std::shared_ptr<xe::Mesh>square = xe::load_mesh_from_obj(std::string(ROOT_DIR) + "/Models/square.obj",
                                              std::string(ROOT_DIR) + "/Models");
    add_submesh(square);

    xe::PointLight light = xe::PointLight(glm::vec3(0.0f, 0.0f, -0.2f), glm::vec3(1.0f, 1.0f, 1.0f), 3.0f, 1.0f);
    add_light(light);

    auto ambient = glm::vec3(1.0f, 0.0f, 0.0f);
    add_ambient(ambient);


    glGenBuffers(1, &lights_ubo_);
    glBindBuffer(GL_UNIFORM_BUFFER, lights_ubo_);

    constexpr GLsizeiptr kHeaderSize       = 32;
    constexpr GLsizeiptr kLightStride      = 48;
    constexpr GLsizeiptr kMaxLights        = 24;
    constexpr GLsizeiptr kTotalLightsBytes = kHeaderSize + kMaxLights * kLightStride;

    glBufferData(GL_UNIFORM_BUFFER, kTotalLightsBytes, nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glBindBufferBase(GL_UNIFORM_BUFFER, 2, lights_ubo_);



    glBindBuffer(GL_UNIFORM_BUFFER, u_pvm_buffer_);
    glBufferData(GL_UNIFORM_BUFFER, 16*sizeof(float) + 16*sizeof(float) + 12*sizeof(float), nullptr, GL_STATIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, u_pvm_buffer_);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glGenVertexArrays(1, &vao_);
    glBindVertexArray(vao_);

    glBindVertexArray(0);

    // Setting the background color of the rendering window,
    // I suggest not to use white or black for better debugging.
    glClearColor(0.81f, 0.81f, 0.8f, 1.0f);

    // This setups an OpenGL viewport of the size of the whole rendering window.
    auto[w, h] = frame_buffer_size();
    glViewport(0, 0, w, h);

}

void SimpleShapeApplication::framebuffer_resize_callback(int w, int h) {
    Application::framebuffer_resize_callback(w, h);
    glViewport(0,0,w,h);
    camera_->set_aspect(static_cast<float>(w) / h);
}


//These functions are called every frame and does the actual rendering.
void SimpleShapeApplication::frame() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    auto PVM = camera_->projection() * camera_->view();
    auto R = glm::mat3(camera_->view());
    auto N = glm::mat3(glm::cross(R[1], R[2]), glm::cross(R[2], R[0]), glm::cross(R[0], R[1]));
    auto Ncol1 = glm::vec4(N[0], 0.0f);
    auto Ncol2 = glm::vec4(N[1], 0.0f);
    auto Ncol3 = glm::vec4(N[2], 0.0f);

    glBindBuffer(GL_UNIFORM_BUFFER, u_pvm_buffer_);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(PVM));
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(camera_->view()));

    const GLsizeiptr baseOffSet = 2 * sizeof(glm::mat4);

    glBufferSubData(GL_UNIFORM_BUFFER, baseOffSet, sizeof(Ncol1), &Ncol1);
    glBufferSubData(GL_UNIFORM_BUFFER, baseOffSet + sizeof(glm::vec4), sizeof(Ncol1), &Ncol2);
    glBufferSubData(GL_UNIFORM_BUFFER, baseOffSet + 2 * sizeof(glm::vec4), sizeof(Ncol1), &Ncol3);

    glBindBuffer(GL_UNIFORM_BUFFER, 0);


    // ---- Lights UBO write (std140) ----
    glBindBuffer(GL_UNIFORM_BUFFER, lights_ubo_);
    //
    constexpr GLsizeiptr kAmbientOffset       = 0;
    constexpr GLsizeiptr kNumLightsOffset     = 16;
    constexpr GLsizeiptr kLightsArrayOffset   = 32;

    constexpr GLsizeiptr kLightStride         = 48;
    constexpr GLsizeiptr kPosVSOffsetInLight  = 0;   // vec4 slot
    constexpr GLsizeiptr kColorOffsetInLight  = 16;  // vec4 slot
    constexpr GLsizeiptr kIntensityOffset     = 32;  // float
    constexpr GLsizeiptr kRadiusOffset        = 36;  // float

    const glm::vec4 ambientStd140(ambient_, 0.0f);
    glBufferSubData(GL_UNIFORM_BUFFER, kAmbientOffset, sizeof(glm::vec4),
                    glm::value_ptr(ambientStd140));

    const GLuint nLights = p_lights_.size();

    glBufferSubData(GL_UNIFORM_BUFFER, kNumLightsOffset, sizeof(GLuint), &nLights);

    // write each light at precise offsets
    const glm::mat4 V = camera_->view();
    for (GLuint i = 0; i < nLights; ++i) {
        auto &light = p_lights_[i];

        // world -> view
        light.position_in_vs = glm::vec3(V * glm::vec4(light.position_in_ws, 1.0f));

        const GLsizeiptr base = kLightsArrayOffset + static_cast<GLsizeiptr>(i) * kLightStride;

        // IMPORTANT: write padded vec3 as vec4 (16 bytes)
        const glm::vec4 posVS4(light.position_in_vs, 0.0f);
        glBufferSubData(GL_UNIFORM_BUFFER, base + kPosVSOffsetInLight,
                        sizeof(glm::vec4), glm::value_ptr(posVS4));


        const glm::vec4 color4(light.color, 0.0f);
        glBufferSubData(GL_UNIFORM_BUFFER, base + kColorOffsetInLight,
                        sizeof(glm::vec4), glm::value_ptr(color4));

        glBufferSubData(GL_UNIFORM_BUFFER, base + kIntensityOffset, sizeof(float), &light.intensity);
        glBufferSubData(GL_UNIFORM_BUFFER, base + kRadiusOffset,    sizeof(float), &light.radius);
    }

    glBindBuffer(GL_UNIFORM_BUFFER, 0);



    for (auto m: meshes_)
        m->draw();

}
