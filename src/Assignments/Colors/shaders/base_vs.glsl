#version 410

layout(location=0) in  vec3 a_vertex_position;
layout(location=1) in  vec3 a_vertex_color;

layout(location=0) out vec3 vertexColor;

void main() {
    gl_Position = vec4(a_vertex_position, 1.0);
    vertexColor = a_vertex_color;
}
