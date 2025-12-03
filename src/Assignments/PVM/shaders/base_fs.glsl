#version 410

layout(location=0) in vec3 vertexColor;
layout(location=0) out vec4 vFragColor;

layout(std140, binding = 0) uniform Modifier {
    float strength;
    vec3  color;
};

void main() {
    vec3 modifiedColor = vertexColor * strength * color;
    vFragColor = vec4(modifiedColor, 1.0);
}
