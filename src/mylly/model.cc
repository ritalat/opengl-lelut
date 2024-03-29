#include "model.hh"

#include "path.hh"

#if defined(__EMSCRIPTEN__) || defined(USE_GLES)
#include "glad/gles2.h"
#else
#include "glad/gl.h"
#endif
#include "glm/glm.hpp"
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

Model::Model(std::string_view file):
    m_VAO(0),
    m_VBO(0)
{
    Path objPath = get_asset_path(file);

    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;

    std::string warn;
    std::string err;

    tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, cpath(objPath), cpath(assetdir()));

    if (!warn.empty())
        fprintf(stderr, "TinyObj warn: %s\n", warn.c_str());

    if (!err.empty())
        throw std::runtime_error("TinyObj err: " + err + FILE_ERROR_HINT);

    for (tinyobj::shape_t &shape : shapes) {
        for (tinyobj::index_t index : shape.mesh.indices) {
            tinyobj::real_t vx = attrib.vertices[3 * index.vertex_index + 0];
            tinyobj::real_t vy = attrib.vertices[3 * index.vertex_index + 1];
            tinyobj::real_t vz = attrib.vertices[3 * index.vertex_index + 2];
            tinyobj::real_t nx = attrib.normals[3 * index.normal_index + 0];
            tinyobj::real_t ny = attrib.normals[3 * index.normal_index + 1];
            tinyobj::real_t nz = attrib.normals[3 * index.normal_index + 2];
            tinyobj::real_t tx = attrib.texcoords[2 * index.texcoord_index + 0];
            tinyobj::real_t ty = attrib.texcoords[2 * index.texcoord_index + 1];

            Vertex vert;
            vert.position.x = vx;
            vert.position.y = vy;
            vert.position.z = vz;
            vert.normal.x = nx;
            vert.normal.y = ny;
            vert.normal.z = nz;
            vert.texcoord.x = tx;
            vert.texcoord.y = ty;

            m_vertices.push_back(vert);
        }
    }

    glGenVertexArrays(1, &m_VAO);
    glBindVertexArray(m_VAO);

    glGenBuffers(1, &m_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);

    glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(Vertex), m_vertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texcoord));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

Model::~Model()
{
    if (m_VAO)
        glDeleteVertexArrays(1, &m_VAO);

    if (m_VBO)
        glDeleteBuffers(1, &m_VBO);
}
