#version 460

layout(location=0) out vec4 vFragColor;

#if __VERSION__ > 410
layout(std140, binding=0) uniform Modifiers {
#else
layout(std140) uniform Color {
    #endif
    vec4  Kd;
    bool use_map_Kd;
};

const int MAX_POINT_LIGHTS=24;

struct PointLight {
    vec3 position_in_view_space;
    vec3 color;
    float intensity;
    float radius;
} ;

layout(std140, binding=2) uniform Lights {
    vec3 ambient;
    uint n_p_lights;
    PointLight p_light[MAX_POINT_LIGHTS];
};

in vec2 vertex_texcoords;
in vec3 vertex_normals_in_vs;
in vec3 vertex_coords_in_vs;

uniform sampler2D map_Kd;

void main() {

    vec3 N = normalize(vertex_normals_in_vs);
    vec3 P = vertex_coords_in_vs;

    vec3 baseColor = Kd.rgb;
    vec3 Lsum = ambient;

    for (uint i = 0u; i < n_p_lights; ++i) {
        PointLight L = p_light[i];
        vec3 toL = normalize(L.position_in_view_space - P);

        float dist = length(toL);
        if (dist > L.radius) {
            continue;
        }

        vec3  Ldir = toL/max(dist, 1e-6);
        float lambertian = max(dot(N, toL), 0.0);

        vec3 Li = L.color * L.intensity * lambertian;

        Lsum += Li;
    }

    vFragColor = vec4(0.1 * ambient * baseColor + 0.25 * Lsum , 1.0f);

}

