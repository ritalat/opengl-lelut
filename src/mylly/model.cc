#include "model.hh"

#include "path.hh"

#if defined(USE_GLES)
#include "glad/gles2.h"
#else
#include "glad/gl.h"
#endif
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/glm.hpp"
#include "glm/gtx/hash.hpp"
#if defined(__ANDROID__)
#include "SDL3/SDL.h"
#endif
#include "tiny_obj_loader.h"

#if defined(__ANDROID__)
#include <sstream>
#endif
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

bool Vertex::operator==(const Vertex &other) const {
    return position == other.position &&
           normal == other.normal &&
           texcoord == other.texcoord;
}

size_t std::hash<Vertex>::operator()(const Vertex &vertex) const {
    return hash<glm::vec3>()(vertex.position) ^
           (hash<glm::vec3>()(vertex.normal) << 1) ^
           (hash<glm::vec2>()(vertex.texcoord) << 2);
}

Model::Model(std::string_view file):
    m_numVertices(0),
    m_numIndices(0),
    m_VAO(0),
    m_VBO(0),
    m_EBO(0)
{
    Path objPath = getAssetPath(file);

    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;

    std::string warn;
    std::string err;

#if defined(__ANDROID__)
    char *objData = (char *)SDL_LoadFile(cpath(objPath), NULL);
    if (!objData)
        throw std::runtime_error("Failed to read file: " + std::string(file));
    std::istringstream strstream(objData);
    tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, &strstream);
    SDL_free(objData);
#else
    tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, cpath(objPath), cpath(assetdir()));
#endif

    if (!warn.empty())
        fprintf(stderr, "TinyObj warn: %s\n", warn.c_str());

    if (!err.empty())
        throw std::runtime_error("TinyObj err: " + err + FILE_ERROR_HINT);

    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::unordered_map<Vertex, unsigned int> uniqueVertices;
    int cvert = 0;
    int cuvert = 0;

    for (const tinyobj::shape_t &shape : shapes) {
        for (const tinyobj::index_t &index : shape.mesh.indices) {
            Vertex vert;
            vert.position.x = attrib.vertices[3 * index.vertex_index + 0];
            vert.position.y = attrib.vertices[3 * index.vertex_index + 1];
            vert.position.z = attrib.vertices[3 * index.vertex_index + 2];
            vert.normal.x = attrib.normals[3 * index.normal_index + 0];
            vert.normal.y = attrib.normals[3 * index.normal_index + 1];
            vert.normal.z = attrib.normals[3 * index.normal_index + 2];
            vert.texcoord.x = attrib.texcoords[2 * index.texcoord_index + 0];
            vert.texcoord.y = attrib.texcoords[2 * index.texcoord_index + 1];

#if 0
            vertices.push_back(vert);
            ++cvert;
#else
            if (uniqueVertices.count(vert) == 0) {
                uniqueVertices[vert] = static_cast<unsigned int>(vertices.size());
                vertices.push_back(vert);
                ++cuvert;
            }
            indices.push_back(uniqueVertices[vert]);
            ++cvert;
#endif
        }
    }

    m_numVertices = static_cast<unsigned int>(vertices.size());
    m_numIndices = static_cast<unsigned int>(indices.size());

    //fprintf(stderr, "Loaded model: %s\n\tTotal vertices: %d\n\tUnique vertices: %d\n", file.data(), cvert, cuvert);

    glGenVertexArrays(1, &m_VAO);
    glBindVertexArray(m_VAO);

    glGenBuffers(1, &m_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, m_numVertices * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texcoord));
    glEnableVertexAttribArray(2);

    glGenBuffers(1, &m_EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_numIndices * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

Model::~Model()
{
    if (m_VAO)
        glDeleteVertexArrays(1, &m_VAO);

    if (m_VBO)
        glDeleteBuffers(1, &m_VBO);

    if (m_EBO)
        glDeleteBuffers(1, &m_EBO);
}

unsigned int Model::vertices() const
{
    return m_numVertices;
}

unsigned int Model::indices() const
{
    return m_numIndices;
}

unsigned int Model::vao() const
{
    return m_VAO;
}
