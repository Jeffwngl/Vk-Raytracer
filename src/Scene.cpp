#include "Scene.h"

void Scene::addSphere(const Sphere& sphere) {
    spheres.push_back(sphere);
    dirty = true;
}

void Scene::addObject(const Object& object) {
    objects.push_back(object);
    dirty = true;
}

uint32_t Scene::addMaterial(const MaterialDefinition& material) {
    for (uint32_t i = 0; i < materials.size(); ++i) {
        if (materials[i] ==  material) {
            return i;
        }
    }

    materials.push_back(material);
    dirty = true;

    return static_cast<uint32_t>(materials.size() - 1);
}

uint32_t Scene::addMesh(Mesh mesh) {
    meshes.push_back(std::move(mesh));
    dirty = true;

    return static_cast<uint32_t>(meshes.size() - 1);
}

Mesh Scene::loadObj(const std::string& path) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> objMaterials;

    std::string warning;
    std::string error;

    if (!tinyobj::LoadObj(
        &attrib,
        &shapes,
        &objMaterials,
        &warning,
        &error,
        path.c_str()
    )) {
        throw std::runtime_error(
            warning + error
        );
    }

    Mesh mesh;

    for (const auto& shape : shapes) {
        for (const auto& index : shape.mesh.indices) {
            Vertex vertex{};

            vertex.position = glm::vec4(
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2],
                1.0f
            );

            if (index.normal_index >= 0) {
                vertex.normal = glm::vec4(
                    attrib.normals[3 * index.normal_index + 0],
                    attrib.normals[3 * index.normal_index + 1],
                    attrib.normals[3 * index.normal_index + 2],
                    0.0f
                );
            }

            if (index.texcoord_index >= 0) {
                vertex.uv = {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
                };
            }

            mesh.vertices.push_back(vertex);

            mesh.indices.push_back(static_cast<uint32_t>(mesh.vertices.size() - 1));
        }
    }

    auto hasNormal = [](const Vertex& v) {
        return glm::dot(
            glm::vec3(v.normal),
            glm::vec3(v.normal)
        ) > 1e-12f;
    };

    // generate missing normals per triangle
    for (size_t i = 0; i < mesh.indices.size(); i += 3) {
        Vertex& v0 = mesh.vertices[mesh.indices[i + 0]];
        Vertex& v1 = mesh.vertices[mesh.indices[i + 1]];
        Vertex& v2 = mesh.vertices[mesh.indices[i + 2]];

        bool missingNormal =
            glm::length(glm::vec3(v0.normal)) == 0.0f ||
            glm::length(glm::vec3(v1.normal)) == 0.0f ||
            glm::length(glm::vec3(v2.normal)) == 0.0f;

        if (
            hasNormal(v0) &&
            hasNormal(v1) &&
            hasNormal(v2)
        ) {
            continue;
        }

        glm::vec3 edge0 = glm::vec3(v1.position - v0.position);

        glm::vec3 edge1 = glm::vec3(v2.position - v0.position);

        glm::vec3 normal = glm::normalize(glm::cross(edge0, edge1));

        v0.normal = glm::vec4(normal, 0.0f);
        v1.normal = glm::vec4(normal, 0.0f);
        v2.normal = glm::vec4(normal, 0.0f);
    }

    return mesh;
}

void Scene::buildBVH() {
    bvh.build(triangles);
}

glm::mat4 getTransformMatrix(const Transform& transform) {
    glm::mat4 translation = glm::translate(
        glm::mat4(1.0f),
        transform.position
    );

    glm::mat4 rotation = glm::mat4_cast(transform.rotation);

    glm::mat4 scale = glm::scale(glm::mat4(1.0f), transform.scale);

    return translation * rotation * scale;
}

Vertex transformVertex(
    const Vertex& vertex,
    const glm::mat4& model,
    const glm::mat3& normalMatrix
) {
    Vertex result = vertex;

    result.position = model * vertex.position;

    result.normal = glm::vec4(
        glm::normalize(normalMatrix * glm::vec3(vertex.normal)),
        0.0f
    );

    return result;
}

void Scene::buildTriangles() {
    triangles.clear();

    for (const Object& object : objects) {
        const Mesh& mesh = meshes[object.meshIndex];

        glm::mat4 model = getTransformMatrix(object.transform);

        glm::mat3 normalMatrix = glm::transpose(
            glm::inverse(glm::mat3(model))
        );

        for (size_t i = 0; i < mesh.indices.size(); i += 3) {
            Vertex v0 = mesh.vertices[mesh.indices[i + 0]];
            Vertex v1 = mesh.vertices[mesh.indices[i + 1]];
            Vertex v2 = mesh.vertices[mesh.indices[i + 2]];

            v0 = transformVertex(v0, model, normalMatrix);
            v1 = transformVertex(v1, model, normalMatrix);
            v2 = transformVertex(v2, model, normalMatrix);

            triangles.push_back({
                .v0 = v0,
                .v1 = v1,
                .v2 = v2,
                .materialIndex = object.materialIndex,
            });
        }
    }

    dirty = true;
}