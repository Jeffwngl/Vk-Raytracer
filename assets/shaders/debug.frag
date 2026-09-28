#version 450

layout(location = 0) in vec3 normal;

layout(location = 0) out vec4 outColor;

void main() {
    vec3 n = normalize(normal);

    outColor = vec4(n * 0.5 + 0.5, 1.0);
}