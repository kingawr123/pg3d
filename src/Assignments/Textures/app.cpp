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

void SimpleShapeApplication::init() {

    xe::ColorMaterial::init();
    set_controller(new CameraController(camera()));

    auto mat_green  = new xe::ColorMaterial(glm::vec4(0.0f, 1.0f, 0.0f, 1.0f)); // green
    auto mat_red    = new xe::ColorMaterial(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f)); // red
    auto mat_blue   = new xe::ColorMaterial(glm::vec4(0.0f, 0.0f, 1.0f, 1.0f)); // blue
    auto mat_purple = new xe::ColorMaterial(glm::vec4(1.0f, 0.0f, 1.0f, 1.0f)); // purple
    auto mat_cyan   = new xe::ColorMaterial(glm::vec4(0.0f, 1.0f, 1.0f, 1.0f)); // cyan


    std::vector<GLfloat> vertices = {
        -0.5f, -0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
         0.0f,  0.5f, 0.5f,

        // second wall
         0.5f, -0.5f, 0.0f,
         0.5f, -0.5f, 1.0f,
         0.0f,  0.5f, 0.5f,

        // third wall
        -0.5f, -0.5f, 1.0f,
         0.5f, -0.5f, 1.0f,
         0.0f,  0.5f, 0.5f,

        // fourth wall
        -0.5f, -0.5f, 0.0f,
        -0.5f, -0.5f, 1.0f,
         0.0f,  0.5f, 0.5f,

        // bottom
        -0.5f, -0.5f, 1.0f,
         0.5f, -0.5f, 1.0f,
         0.5f, -0.5f, 0.0f,
        -0.5f, -0.5f, 0.0f,
    };

    std::vector<GLushort> indexes = {
        0,  2,  1,     // first wall
        3,  5,  4,     // second wall
        6,  7,  8,     // third wall
        9, 10, 11,     // fourth wall
        12, 14, 13,    // bottom
        12, 15, 14     // bottom
    };


    auto pyramid = new xe::Mesh;

    pyramid->allocate_vertex_buffer(vertices.size() * sizeof(GLfloat), GL_STATIC_DRAW);
    pyramid->load_vertices(0, vertices.size() * sizeof(GLfloat), vertices.data());
    pyramid->vertex_attrib_pointer(0, 3, GL_FLOAT, 3 * sizeof(GLfloat), 0);

    pyramid->allocate_index_buffer(indexes.size() * sizeof(GLushort), GL_STATIC_DRAW);
    pyramid->load_indices(0, indexes.size() * sizeof(GLushort), indexes.data());

    pyramid->add_submesh( 0,  3, mat_green);   // first wall
    pyramid->add_submesh( 3,  6, mat_red);     // second wall
    pyramid->add_submesh( 6,  9, mat_blue);    // third wall
    pyramid->add_submesh( 9, 12, mat_purple);  // fourth wall
    pyramid->add_submesh(12, 18, mat_cyan);    // bottom (2 triangles)

    add_submesh(pyramid);

    // modifier data:
    float strength = 1.0f;
    float color[3] = {1.0f, 1.0f, 1.0f};

    float modifier_uniform_data[8] = {0};
    modifier_uniform_data[0] = strength;
    modifier_uniform_data[4] = color[0];
    modifier_uniform_data[5] = color[1];
    modifier_uniform_data[6] = color[2];


    // uniform modifier buffer
    GLuint modifier_u_buffer_handle;
    glGenBuffers(1, &modifier_u_buffer_handle);
    glBindBuffer(GL_UNIFORM_BUFFER, modifier_u_buffer_handle);
    glBufferData(GL_UNIFORM_BUFFER, 8*sizeof(float), nullptr, GL_STATIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, modifier_u_buffer_handle);

    // uniform transform PVM buffer
    glBindBuffer(GL_UNIFORM_BUFFER, u_pvm_buffer_);
    glBufferData(GL_UNIFORM_BUFFER, 16*sizeof(float), nullptr, GL_STATIC_DRAW); // 16 * sizeof(float) = 64
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, u_pvm_buffer_);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    // This setups a Vertex Array Object (VAO) that  encapsulates
    // the state of all vertex buffers needed for rendering
    glGenVertexArrays(1, &vao_);
    glBindVertexArray(vao_);

    // ----------- uniform modifier ---------
    glBindBuffer(GL_UNIFORM_BUFFER, modifier_u_buffer_handle);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(modifier_uniform_data), modifier_uniform_data);


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
