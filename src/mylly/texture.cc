#include "texture.hh"

#include "path.hh"

#if defined(USE_GLES)
#include "glad/gles2.h"
#else
#include "glad/gl.h"
#endif
#if defined(__ANDROID__)
#include "SDL3/SDL.h"
#endif
#include "stb_image.h"

#include <array>
#include <stdexcept>
#include <string_view>

Texture::Texture(std::string_view file, bool flip):
    m_id(0)
{
    stbi_set_flip_vertically_on_load(flip);

    glGenTextures(1, &m_id);
    glBindTexture(GL_TEXTURE_2D, m_id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    auto channelsToFormat = [](int n){
        if (n == 1) {
            return GL_RED;
        } else if (n == 3) {
            return GL_RGB;
        } else {
            return GL_RGBA;
        }
    };

    int width, height, nrChannels;
    Path texturePath = getAssetPath(file);

#if defined(__ANDROID__)
    size_t imgSize;
    unsigned char *imgData = (unsigned char *)SDL_LoadFile(cpath(texturePath), &imgSize);
    if (!imgData)
        throw std::runtime_error("Failed to read file: " + std::string(file));
    unsigned char *data = stbi_load_from_memory(imgData, (int)imgSize, &width, &height, &nrChannels, 0);
    SDL_free(imgData);
#else
    unsigned char *data = stbi_load(cpath(texturePath), &width, &height, &nrChannels, 0);
#endif

    if (!data)
        throw std::runtime_error("Failed to load texture: " + texturePath.string() + FILE_ERROR_HINT);

    GLenum format = channelsToFormat(nrChannels);
    glTexImage2D(GL_TEXTURE_2D, 0, format == GL_RED ? GL_R8 : format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);
}

Texture::~Texture()
{
    if (m_id)
        glDeleteTextures(1, &m_id);
}

void Texture::activate(int unit)
{
    if (unit < 48) {
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_2D, m_id);
    }
}

unsigned int Texture::id() const
{
    return m_id;
}

void Texture::filtering(int mag, int min)
{
    glBindTexture(GL_TEXTURE_2D, m_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::wrapping(int s, int t)
{
    glBindTexture(GL_TEXTURE_2D, m_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, s);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, t);
    glBindTexture(GL_TEXTURE_2D, 0);
}

// Cubemap face order:
// 1. Right  (POSITIVE_X)
// 2. Left   (NEGATIVE_X)
// 3. Top    (POSITIVE_Y)
// 4. Bottom (NEGATIVE_Y)
// 5. Back   (POSITIVE_Z)
// 6. front  (NEGATIVE_Z)
Cubemap::Cubemap(const std::array<std::string_view, 6> &files):
    m_id(0)
{
    glGenTextures(1, &m_id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_id);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    for (size_t i = 0; i < files.size(); ++i) {
        int width, height, nrChannels;
        Path texturePath = getAssetPath(files[i]);

#if defined(__ANDROID__)
        size_t imgSize;
        unsigned char *imgData = (unsigned char *)SDL_LoadFile(cpath(texturePath), &imgSize);
        if (!imgData)
            throw std::runtime_error("Failed to read file: " + std::string(files[i]));
        unsigned char *data = stbi_load_from_memory(imgData, (int)imgSize, &width, &height, &nrChannels, 0);
        SDL_free(imgData);
#else
        unsigned char *data = stbi_load(cpath(texturePath), &width, &height, &nrChannels, 0);
#endif

        if (!data)
            throw std::runtime_error("Failed to load texture: " + texturePath.string() + FILE_ERROR_HINT);

        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + static_cast<int>(i), 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        stbi_image_free(data);
    }
}

Cubemap::~Cubemap()
{
    if (m_id)
        glDeleteTextures(1, &m_id);
}

void Cubemap::activate(int unit)
{
    if (unit < 48) {
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_CUBE_MAP, m_id);
    }
}

unsigned int Cubemap::id() const
{
    return m_id;
}

void Cubemap::filtering(int mag, int min)
{
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_id);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, mag);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, min);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

void Cubemap::wrapping(int s, int t, int r)
{
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_id);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, s);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, t);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, r);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}
