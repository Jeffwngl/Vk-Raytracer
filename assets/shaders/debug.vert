#version 450

struct Vertex {
    vec4 position;
    vec4 normal;
    vec2 uv;
    vec2 padding;
};

struct Triangle {
    Vertex v0;
    Vertex v1;
    Vertex v2;

    uint materialIndex;
    uint pad0;
    uint pad1;
    uint pad2;
};

layout(std430, set = 0, binding = 0)
readonly buffer TriangleBuffer {
    Triangle triangles[];
};

layout(push_constant) uniform PushConstants {
    mat4 viewProjection;
} pc;

layout(location = 0) out vec3 outNormal;

void main() {
    uint triangleIndex = uint(gl_VertexIndex) / 3u;
    uint vertexIndex = uint(gl_VertexIndex) % 3u;

    Vertex vertex;

    if (vertexIndex == 0u) {
        vertex = triangles[triangleIndex].v0;
    }
    else if (vertexIndex == 1u) {
        vertex = triangles[triangleIndex].v1;
    }
    else {
        vertex = triangles[triangleIndex].v2;
    }

    gl_Position = pc.viewProjection * vec4(vertex.position.xyz, 1.0);

    outNormal = vertex.normal.xyz;
}