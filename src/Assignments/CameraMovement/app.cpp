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
#include "glm/ext/scalar_constants.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

void SimpleShapeApplication::init() {
    // A utility function that reads the shader sources, compiles them and creates the program object
    // As everything in OpenGL we reference program by an integer "handle".
    auto program = xe::utils::create_program(
            {{GL_VERTEX_SHADER,   std::string(PROJECT_DIR) + "/shaders/base_vs.glsl"},
             {GL_FRAGMENT_SHADER, std::string(PROJECT_DIR) + "/shaders/base_fs.glsl"}});

    if (!program) {
        std::cerr << "Invalid program" << std::endl;
        exit(-1);
    }

    // A vector containing the x,y,z, r, g, b vertex coordinates for shapes.
    std::vector<GLfloat> vertices = {
        // new pyramid
        // first wall
        -0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f, // green
        0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,
        0.0f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f,

        // second wall
        0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f, // red
        0.5f, -0.5f, 1.0f,  1.0f, 0.0f, 0.0f,
        0.0f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f,

        // 3 wall
        -0.5f, -0.5f, 1.0f,  0.0f, 0.0f, 1.0f, // blue
        0.5f, -0.5f, 1.0f,  0.0f, 0.0f, 1.0f,
        0.0f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f,

        // 4 wall
        -0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, 1.0f,  1.0f, 0.0f, 1.0f, // purple
        0.0f, 0.5f, 0.5f, 1.0f, 0.0f, 1.0f,

        // bottom
        -0.5f, -0.5f, 1.0f,  0.0f, 1.0f, 1.0f,
        0.5f, -0.5f, 1.0f,  0.0f, 1.0f, 1.0f,
        0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 1.0f,
        -0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 1.0f,
    };


    std::vector<GLushort> indexes = {
        0, 1, 2,
        3, 5, 4,
        6, 7, 8,
        9, 10, 11,
        12, 14, 13,
        12, 15, 14

    };

    // modifier data:
    float strength = 1.0f;
    float color[3] = {1.0f, 1.0f, 1.0f};

    float modifier_uniform_data[8] = {0};
    modifier_uniform_data[0] = strength;
    modifier_uniform_data[4] = color[0];
    modifier_uniform_data[5] = color[1];
    modifier_uniform_data[6] = color[2];

    // // transformations PVM data:
    // auto PVM = camera_->projection() * camera_->view();

    // Buffer with indexes
    GLuint i_buffer_handle;
    glGenBuffers(1, &i_buffer_handle);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, i_buffer_handle);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexes.size() * sizeof(GLushort), indexes.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // Generating the buffer and loading the vertex data into it.
    GLuint v_buffer_handle;
    glGenBuffers(1, &v_buffer_handle);
    OGL_CALL(glBindBuffer(GL_ARRAY_BUFFER, v_buffer_handle));
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

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


    // ----------- uniform transform ---------
    // glBindBuffer(GL_UNIFORM_BUFFER, u_pvm_buffer_);
    //
    // glBufferSubData(GL_UNIFORM_BUFFER, 0,  sizeof(PVM[0]), &PVM[0]);
    // glBufferSubData(GL_UNIFORM_BUFFER, 16, sizeof(PVM[1]), &PVM[1]);
    // glBufferSubData(GL_UNIFORM_BUFFER, 32, sizeof(PVM[2]), &PVM[2]);
    // glBufferSubData(GL_UNIFORM_BUFFER, 48, sizeof(PVM[3]), &PVM[3]);


    // -----------  indexes  ----------------
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, i_buffer_handle);


    // ----------- vertices ----------------
    glBindBuffer(GL_ARRAY_BUFFER, v_buffer_handle);

    // This indicates that the data for attribute 0 should be read from a vertex buffer.
    glEnableVertexAttribArray(0);
    // and this specifies how the data is layout in the buffer.
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), reinterpret_cast<GLvoid *>(0));

    // colors rgb
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), reinterpret_cast<GLvoid *>(3 * sizeof(GLfloat)));

    glBindBuffer(GL_ARRAY_BUFFER, 0);


    glBindVertexArray(0);
    //end of vao "recording"

    // Setting the background color of the rendering window,
    // I suggest not to use white or black for better debuging.
    glClearColor(0.81f, 0.81f, 0.8f, 1.0f);

    // This setups an OpenGL vieport of the size of the whole rendering window.
    auto[w, h] = frame_buffer_size();
    glViewport(0, 0, w, h);

    glUseProgram(program);
}

void SimpleShapeApplication::framebuffer_resize_callback(int w, int h) {
    Application::framebuffer_resize_callback(w, h);
    glViewport(0,0,w,h);
    camera_->set_aspect(static_cast<float>(w) / h);
}

//This functions is called every frame and does the actual rendering.
void SimpleShapeApplication::frame() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    auto PVM = camera_->projection() * camera_->view();
    glBindBuffer(GL_UNIFORM_BUFFER, u_pvm_buffer_);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(PVM));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    // Binding the VAO will setup all the required vertex buffers.
    glBindVertexArray(vao_);
    glDrawElements(GL_TRIANGLES, 25, GL_UNSIGNED_SHORT, nullptr);
    glBindVertexArray(0);
}
