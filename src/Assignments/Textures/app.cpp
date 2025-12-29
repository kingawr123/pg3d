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
#include "Engine/Material.h"
#include "Engine/mesh_loader.h"

#define STB_IMAGE_IMPLEMENTATION  1

#include "3rdParty/stb/stb_image.h"

void SimpleShapeApplication::init() {

    xe::ColorMaterial::init();
    set_controller(new CameraController(camera()));

    xe::Mesh *pyramid_v2 = xe::load_mesh_from_obj(std::string(ROOT_DIR) + "/Models/pyramid.obj",
                                                  std::string(ROOT_DIR) + "/Models");
    add_submesh(pyramid_v2);

    // uniform transform PVM buffer
    glBindBuffer(GL_UNIFORM_BUFFER, u_pvm_buffer_);
    glBufferData(GL_UNIFORM_BUFFER, 16*sizeof(float), nullptr, GL_STATIC_DRAW); // 16 * sizeof(float) = 64
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
    glBindBuffer(GL_UNIFORM_BUFFER, u_pvm_buffer_);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(PVM));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    for (auto m: meshes_)
        m->draw();

}
