#include "Shader.h"

#include <glad/gl.h>

#include <cstdio>
#include <vector>

namespace AIForge {

namespace {

bool CompileStage(GLenum type, const std::string& src, GLuint& outId,
                  const char* tag) {
    outId = glCreateShader(type);
    const char* csrc = src.c_str();
    glShaderSource(outId, 1, &csrc, nullptr);
    glCompileShader(outId);
    GLint ok = 0;
    glGetShaderiv(outId, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        GLint len = 0;
        glGetShaderiv(outId, GL_INFO_LOG_LENGTH, &len);
        std::vector<char> log(len > 0 ? len : 1);
        glGetShaderInfoLog(outId, len, nullptr, log.data());
        std::fprintf(stderr, "[Shader] %s compile failed:\n%s\n", tag,
                     log.data());
        glDeleteShader(outId);
        outId = 0;
        return false;
    }
    return true;
}

}  // namespace

Shader::Shader() = default;
Shader::~Shader() { Destroy(); }

bool Shader::LoadFromSource(const std::string& vsSource,
                            const std::string& fsSource) {
    Destroy();

    GLuint vs = 0, fs = 0;
    if (!CompileStage(GL_VERTEX_SHADER, vsSource, vs, "vertex")) return false;
    if (!CompileStage(GL_FRAGMENT_SHADER, fsSource, fs, "fragment")) {
        glDeleteShader(vs);
        return false;
    }

    m_program = glCreateProgram();
    glAttachShader(m_program, vs);
    glAttachShader(m_program, fs);
    glLinkProgram(m_program);
    glDeleteShader(vs);
    glDeleteShader(fs);

    GLint ok = 0;
    glGetProgramiv(m_program, GL_LINK_STATUS, &ok);
    if (!ok) {
        GLint len = 0;
        glGetProgramiv(m_program, GL_INFO_LOG_LENGTH, &len);
        std::vector<char> log(len > 0 ? len : 1);
        glGetProgramInfoLog(m_program, len, nullptr, log.data());
        std::fprintf(stderr, "[Shader] link failed:\n%s\n", log.data());
        glDeleteProgram(m_program);
        m_program = 0;
        return false;
    }
    return true;
}

void Shader::Destroy() {
    if (m_program) {
        glDeleteProgram(m_program);
        m_program = 0;
    }
    m_uniformCache.clear();
}

void Shader::Bind() const {
    if (m_program) glUseProgram(m_program);
}

void Shader::Unbind() const { glUseProgram(0); }

int Shader::GetUniformLocation(const std::string& name) {
    auto it = m_uniformCache.find(name);
    if (it != m_uniformCache.end()) return it->second;
    int loc = glGetUniformLocation(m_program, name.c_str());
    if (loc < 0) {
        std::fprintf(stderr, "[Shader] WARN: uniform '%s' not found\n",
                     name.c_str());
    }
    m_uniformCache[name] = loc;
    return loc;
}

void Shader::SetInt(const std::string& name, int v) {
    Bind();
    int loc = GetUniformLocation(name);
    if (loc >= 0) glUniform1i(loc, v);
}

void Shader::SetFloat(const std::string& name, float v) {
    Bind();
    int loc = GetUniformLocation(name);
    if (loc >= 0) glUniform1f(loc, v);
}

void Shader::SetVec2(const std::string& name, const Vec2& v) {
    Bind();
    int loc = GetUniformLocation(name);
    if (loc >= 0) glUniform2f(loc, v.x, v.y);
}

void Shader::SetVec3(const std::string& name, const Vec3& v) {
    Bind();
    int loc = GetUniformLocation(name);
    if (loc >= 0) glUniform3f(loc, v.x, v.y, v.z);
}

void Shader::SetVec4(const std::string& name, const Vec4& v) {
    Bind();
    int loc = GetUniformLocation(name);
    if (loc >= 0) glUniform4f(loc, v.x, v.y, v.z, v.w);
}

void Shader::SetMat4(const std::string& name, const Mat4& v) {
    Bind();
    int loc = GetUniformLocation(name);
    if (loc >= 0) glUniformMatrix4fv(loc, 1, GL_FALSE, v.Data());
}

}  // namespace AIForge
