#pragma once

#include "api_decl.hh"

#include "glm/glm.hpp"

#include <string_view>

struct Vertex
{
    bool operator==(const Vertex &other) const;
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texcoord;
};

template <>
struct std::hash<Vertex> {
    size_t operator()(const Vertex &vertex) const;
};

class MYLLY_API Model
{
public:
    Model(std::string_view file);
    ~Model();
    Model(const Model &) = delete;
    Model &operator=(const Model &) = delete;
    unsigned int vertices() const;
    unsigned int indices() const;
    unsigned int vao() const;

private:
    unsigned int m_numVertices;
    unsigned int m_numIndices;
    unsigned int m_VAO;
    unsigned int m_VBO;
    unsigned int m_EBO;
};
