#include "Framebuffer.h"

#include <glad/gl.h>

#include <cstdio>

namespace AIForge {

Framebuffer::~Framebuffer() { Destroy(); }

bool Framebuffer::Create(int width, int height) {
    Destroy();
    m_width  = width  > 0 ? width  : 1;
    m_height = height > 0 ? height : 1;

    glGenFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    // 颜色附件:一张 RGBA8 纹理
    glGenTextures(1, &m_colorTex);
    glBindTexture(GL_TEXTURE_2D, m_colorTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_width, m_height, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                           m_colorTex, 0);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    if (status != GL_FRAMEBUFFER_COMPLETE) {
        std::fprintf(stderr, "[Framebuffer] incomplete (status=0x%X)\n", status);
        Destroy();
        return false;
    }
    return true;
}

void Framebuffer::Resize(int width, int height) {
    if (width == m_width && height == m_height) return;
    Create(width, height);
}

void Framebuffer::Destroy() {
    if (m_colorTex) {
        glDeleteTextures(1, &m_colorTex);
        m_colorTex = 0;
    }
    if (m_fbo) {
        glDeleteFramebuffers(1, &m_fbo);
        m_fbo = 0;
    }
    m_width = m_height = 0;
}

void Framebuffer::Bind() const {
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glViewport(0, 0, m_width, m_height);
}

void Framebuffer::BindDefault(int screenWidth, int screenHeight) {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, screenWidth, screenHeight);
}

void Framebuffer::BindColorTexture(int unit) const {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, m_colorTex);
}

}  // namespace AIForge
