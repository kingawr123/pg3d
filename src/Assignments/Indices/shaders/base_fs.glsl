#version 410

layout(location=0) in vec3 vertexColor;
layout(location=0) out vec4 vFragColor;

void main() {
    vFragColor = vec4(vertexColor, 1.0);
}
