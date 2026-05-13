#include "Texture.h"

#include <glad/gl.h>

#include <cstdio>

namespace AIForge {

Texture::Texture() = default;
Texture::~Texture() { Destroy(); }

bool Texture::CreateFromRGBA(int width, int height, const uint8_t* pixels,
                             bool useMipmap, bool linearFilter) {
    Destroy();
    if (width <= 0 || height <= 0 || !pixels) {
        std::fprintf(stderr, "[Texture] invalid input %dx%d\n", width, height);
        return false;
    }

    glGenTextures(1, &m_id);
    glBindTexture(GL_TEXTURE_2D, m_id);

    GLenum minFilter = linearFilter
        ? (useMipmap ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR)
        : (useMipmap ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST);
    GLenum magFilter = linearFilter ? GL_LINEAR : GL_NEAREST;

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, pixels);

    if (useMipmap) glGenerateMipmap(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, 0);

    m_width  = width;
    m_height = height;
    return true;
}

void Texture::Destroy() {
    if (m_id) {
        glDeleteTextures(1, &m_id);
        m_id = 0;
    }
    m_width = m_height = 0;
}

void Texture::Bind(int unit) const {
    if (!m_id) return;
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, m_id);
}

}  // namespace AIForge
