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
        // Triangle (roof) - red
        -0.5f,  0.0f, 0.0f,  1.0f, 0.0f, 0.0f,
         0.0f,  0.5f, 0.0f,  1.0f, 0.0f, 0.0f,
         0.5f,  0.0f, 0.0f,  1.0f, 0.0f, 0.0f,

        // Rectangle (walls) - green
        -0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,
         0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,
        -0.5f,  0.0f, 0.0f,  0.0f, 1.0f, 0.0f,
         0.5f,  0.0f, 0.0f,  0.0f, 1.0f, 0.0f
    };


    std::vector<GLushort> indexes = {
        0, 1, 2,
        5, 3, 6,
        6, 4, 3
    };

    // modifier data:
    float strength = 0.5f;
    float color[3] = {1.0f, 1.0f, 1.0f};

    float modifier_uniform_data[8] = {0};
    modifier_uniform_data[0] = strength;
    modifier_uniform_data[4] = color[0];
    modifier_uniform_data[5] = color[1];
    modifier_uniform_data[6] = color[2];

    // transform data:
    float theta = 1.0*glm::pi<float>() / 6.0f; // 30°
    float cs = std::cos(theta);
    float ss = std::sin(theta);
    glm::mat2 rot{cs, ss, -ss, cs};
    glm::vec2 trans{0.0f, -0.25f};
    glm::vec2 scale{0.5f, 0.5f};

    float transform_uniform_data[12] = {0}; // 12 floatów = 48 B

    // scale: offset 0 B -> float 0..1
    transform_uniform_data[0] = scale.x;
    transform_uniform_data[1] = scale.y;

    // translation: offset 8 B -> float 2..3
    transform_uniform_data[2] = trans.x;
    transform_uniform_data[3] = trans.y;

    // rotation: offset 16 B -> a, b w floatach 4..7 (6..7 padding)
    transform_uniform_data[4] = rot[0].x;  // cs
    transform_uniform_data[5] = rot[0].y;  // ss

    // rotation: offset 32 B -> c, d w floatach 8..11 (10..11 padding)
    transform_uniform_data[8]  = rot[1].x; // -ss
    transform_uniform_data[9]  = rot[1].y; //  cs


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

    // uniform transform buffer
    GLuint transform_u_buffer_handle;
    glGenBuffers(1, &transform_u_buffer_handle);
    glBindBuffer(GL_UNIFORM_BUFFER, transform_u_buffer_handle);
    glBufferData(GL_UNIFORM_BUFFER, 12*sizeof(float), nullptr, GL_STATIC_DRAW); // 12 * sizeof(floa) = 48
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, transform_u_buffer_handle);

    // This setups a Vertex Array Object (VAO) that  encapsulates
    // the state of all vertex buffers needed for rendering
    glGenVertexArrays(1, &vao_);
    glBindVertexArray(vao_);

    // ----------- uniform modifier ---------

    glBindBuffer(GL_UNIFORM_BUFFER, modifier_u_buffer_handle);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(modifier_uniform_data), modifier_uniform_data);

    // ----------- uniform transform ---------

    glBindBuffer(GL_UNIFORM_BUFFER, transform_u_buffer_handle);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(transform_uniform_data), transform_uniform_data);

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

//This functions is called every frame and does the actual rendering.
void SimpleShapeApplication::frame() {
    // Binding the VAO will setup all the required vertex buffers.
    glBindVertexArray(vao_);
    glDrawElements(GL_TRIANGLES, 9, GL_UNSIGNED_SHORT, nullptr);
    glBindVertexArray(0);
}
