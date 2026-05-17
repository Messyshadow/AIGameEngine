#include "Texture.h"

#include <glad/gl.h>
#include <SDL3/SDL_filesystem.h>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_FAILURE_USERMSG
#include <stb_image.h>

#include <cstdio>
#include <filesystem>

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

bool Texture::CreateFromFile(const std::string& path, bool useMipmap,
                             bool linearFilter) {
    stbi_set_flip_vertically_on_load(1);   // 让 (0,0) 在左下,匹配 GL 习惯

    int w = 0, h = 0, ch = 0;
    unsigned char* data = nullptr;
    std::string usedPath;

    // 尝试 1:相对 .exe 所在目录(最可靠 — data/ 一定被 POST_BUILD 复制到这)
    const char* base = SDL_GetBasePath();   // SDL3 拥有所有权,勿 free
    if (base) {
        std::string full = std::string(base) + path;
        data = stbi_load(full.c_str(), &w, &h, &ch, 4);
        if (data) usedPath = full;
    }

    // 尝试 2:直接按相对/绝对路径(相对于当前工作目录)
    if (!data) {
        data = stbi_load(path.c_str(), &w, &h, &ch, 4);
        if (data) usedPath = path;
    }

    if (!data) {
        std::error_code ec;
        std::string cwd = std::filesystem::current_path(ec).string();
        const char* base = SDL_GetBasePath();
        std::fprintf(stderr,
            "[Texture] FAIL to load '%s'\n"
            "  reason   : %s\n"
            "  CWD      : %s\n"
            "  exe dir  : %s\n"
            "  tried    : (1) <CWD>/%s   (2) <exe_dir>/%s\n"
            "  hint     : put 'data/' next to the .exe or run from project root\n",
            path.c_str(), stbi_failure_reason(), cwd.c_str(),
            base ? base : "(null)", path.c_str(), path.c_str());
        return false;
    }

    bool ok = CreateFromRGBA(w, h, data, useMipmap, linearFilter);
    stbi_image_free(data);
    if (ok) {
        std::printf("[Texture] loaded '%s' (%dx%d) via %s\n",
                    path.c_str(), w, h, usedPath.c_str());
    }
    return ok;
}

void Texture::Bind(int unit) const {
    if (!m_id) return;
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, m_id);
}

}  // namespace AIForge
