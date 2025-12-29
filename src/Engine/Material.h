//
// Created by Piotr Białas on 02/11/2021.
//

#pragma once

#include <glad/gl.h>
#include <glm/glm.hpp>

#include "Application/utils.h"

namespace xe {

    class Material {
    public:
        virtual void bind() = 0;

        virtual void unbind() {};
    };

    class ColorMaterial : public Material {
    public:
        // Textures
        ColorMaterial(const glm::vec4 color, GLuint texture, GLuint texture_unit) : color_(color), texture_(texture),
                                                                                    texture_unit_(texture_unit) {}

        ColorMaterial(const glm::vec4 color, GLuint texture) : ColorMaterial(color, texture, 0) {}

        ColorMaterial(const glm::vec4 color) : ColorMaterial(color, 0) {}


        void set_texture(GLuint tex) { texture_ = tex; }
        GLuint get_texture() { return texture_; }

        void set_texture_unit(GLuint unit) { texture_unit_ = unit; }
        GLuint get_texture_unit() { return texture_unit_; }
        // ----------

        void bind();

        void unbind();

        static void init();

        static GLuint program() { return shader_; }

    private:
        static GLuint shader_;
        static GLuint color_uniform_buffer_;

        glm::vec4 color_;

        // Textures
        static GLint uniform_map_Kd_location_;
        GLuint texture_;
        GLuint texture_unit_;

    };

    GLuint create_texture(const std::string &name);

}


