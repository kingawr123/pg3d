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


struct Std140PointLight {
    Std140Vec3 position_in_view_space;
    Std140Vec3 color;
    float intensity;
    float radius;
    float _pad0 = 0.0f;
};

struct Std140LightsHeader {
    Std140Vec3 ambient;
    uint32_t n_p_lights;
    uint32_t _pad0 = 0;
};

void SimpleShapeApplication::init() {

    xe::ColorMaterial::init();
    xe::PhongMaterial::init();

    set_controller(new CameraController(camera()));

    xe::Mesh *square = xe::load_mesh_from_obj(std::string(ROOT_DIR) + "/Models/square.obj",
                                              std::string(ROOT_DIR) + "/Models");
    add_submesh(square);

    xe::PointLight light = xe::PointLight(glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(1.0f, 1.0f, 1.0f), 1.0f, 1.0f);
    add_light(light);

    auto ambient = glm::vec3(1.0f, 0.0f, 0.0f);
    add_ambient(ambient);

    glGenBuffers(1, &lights_ubo_);
    glBindBuffer(GL_UNIFORM_BUFFER, lights_ubo_);

    const size_t headerSize = sizeof(Std140LightsHeader);
    const size_t lightsSize = 24 * sizeof(Std140PointLight);
    const size_t totalSize  = headerSize + lightsSize;

    glBufferData(GL_UNIFORM_BUFFER, totalSize, nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glBindBufferBase(GL_UNIFORM_BUFFER, 2, lights_ubo_);


    // uniform transform PVM buffer
    glBindBuffer(GL_UNIFORM_BUFFER, u_pvm_buffer_);
    glBufferData(GL_UNIFORM_BUFFER, 16*sizeof(float) + 16*sizeof(float) + 12*sizeof(float), nullptr, GL_STATIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, u_pvm_buffer_);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    // This setups a Vertex Array Object (VAO) that  encapsulates
    // the state of all vertex buffers needed for rendering
    glGenVertexArrays(1, &vao_);
    glBindVertexArray(vao_);


    glBindVertexArray(0);
    //end of vao "recording"

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

    // Lights

    constexpr GLsizeiptr kNumLightsOffset     = 16;   // uint (4 bytes)
    constexpr GLsizeiptr kLightsArrayOffset   = 32;   // next 16-aligned slot after header
    constexpr GLsizeiptr kLightStride         = 48;   // sizeof(PointLight in std140)
    constexpr GLsizeiptr kPosVSOffsetInLight  = 0;    // vec3 padded → write as vec4
    constexpr GLsizeiptr kColorOffsetInLight  = 16;   // vec3 padded → write as vec4
    constexpr GLsizeiptr kIntensityOffset     = 32;   // float
    constexpr GLsizeiptr kRadiusOffset        = 36;   // float

    glBindBuffer(GL_UNIFORM_BUFFER, lights_ubo_);
    const glm::vec4 ambientStd140(ambient_, 0.0f);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::vec4), glm::value_ptr(ambientStd140));

    const GLuint lights_size = p_lights_.size();
    glBufferSubData(GL_UNIFORM_BUFFER, kNumLightsOffset, sizeof(GLuint), &lights_size);


    // per-light data
    for (GLuint i = 0; i < lights_size; ++i) {
        auto &light = p_lights_[i];

        // world space -> view space
        const glm::vec3 posVS = glm::vec3(camera_->view() * glm::vec4(light.position_in_ws, 1.0f));
        // optionally store back into your CPU struct
        light.position_in_vs = posVS;

        // base offset for light i
        const GLsizeiptr base = kLightsArrayOffset + static_cast<GLsizeiptr>(i) * kLightStride;

        glBufferSubData(GL_UNIFORM_BUFFER, base + kPosVSOffsetInLight, sizeof(glm::vec4), glm::value_ptr(light.position_in_vs));

        glBufferSubData(GL_UNIFORM_BUFFER, base + kColorOffsetInLight, sizeof(glm::vec4), glm::value_ptr(light.color));

        // intensity and radius
        glBufferSubData(GL_UNIFORM_BUFFER, base + kIntensityOffset, sizeof(float), &light.intensity);
        glBufferSubData(GL_UNIFORM_BUFFER, base + kRadiusOffset,    sizeof(float), &light.radius);
    }

    glBindBuffer(GL_UNIFORM_BUFFER, 0);


    // std::vector<Std140PointLight> packed;
    // packed.resize(lights_size);
    //
    // for (GLuint i = 0; i < lights_size; ++i) {
    //     auto &light = p_lights_[i];
    //     Std140PointLight L{};
    //
    //     light.position_in_vs = glm::vec3(camera_->view() * glm::vec4(light.position_in_ws, 1.0f));
    //     L.position_in_view_space.v = light.position_in_vs;
    //     L.color.v = light.color;
    //     L.intensity = light.intensity;
    //     L.radius = light.radius;
    //
    //     packed[i] = L;
    // }
    //
    // if (!packed.empty()) {
    //     glBufferSubData(GL_UNIFORM_BUFFER,
    //                     32,
    //                     static_cast<GLsizeiptr>(packed.size() * sizeof(Std140PointLight)),
    //                     packed.data());
    // }



    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    for (auto m: meshes_)
        m->draw();

}
