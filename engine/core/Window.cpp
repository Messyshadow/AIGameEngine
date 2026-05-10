#include "Window.h"

#include <glad/gl.h>
#include <SDL3/SDL.h>

#include <cstdio>

namespace AIForge {

namespace {
bool s_sdlInited = false;
}

Window::Window() = default;

Window::~Window() { Shutdown(); }

bool Window::Init(const Config& cfg) {
    if (m_initialized) return true;

    if (!s_sdlInited) {
        if (!SDL_Init(SDL_INIT_VIDEO)) {
            std::fprintf(stderr, "[Window] SDL_Init failed: %s\n", SDL_GetError());
            return false;
        }
        s_sdlInited = true;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    SDL_WindowFlags flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;
    if (cfg.fullscreen) flags |= SDL_WINDOW_FULLSCREEN;

    m_window = SDL_CreateWindow(cfg.title.c_str(), cfg.width, cfg.height, flags);
    if (!m_window) {
        std::fprintf(stderr, "[Window] SDL_CreateWindow failed: %s\n",
                     SDL_GetError());
        return false;
    }

    m_glContext = SDL_GL_CreateContext(m_window);
    if (!m_glContext) {
        std::fprintf(stderr, "[Window] SDL_GL_CreateContext failed: %s\n",
                     SDL_GetError());
        SDL_DestroyWindow(m_window);
        m_window = nullptr;
        return false;
    }

    if (!SDL_GL_MakeCurrent(m_window, m_glContext)) {
        std::fprintf(stderr, "[Window] SDL_GL_MakeCurrent failed: %s\n",
                     SDL_GetError());
        Shutdown();
        return false;
    }

    SDL_GL_SetSwapInterval(cfg.vsync ? 1 : 0);

    int version =
        gladLoadGL(reinterpret_cast<GLADloadfunc>(SDL_GL_GetProcAddress));
    if (version == 0) {
        std::fprintf(stderr, "[Window] gladLoadGL failed\n");
        Shutdown();
        return false;
    }

    m_width = cfg.width;
    m_height = cfg.height;
    m_initialized = true;

    std::printf("[Window] %dx%d  OpenGL %d.%d  vsync=%d\n", m_width, m_height,
                GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version),
                cfg.vsync ? 1 : 0);
    return true;
}

void Window::Shutdown() {
    if (m_glContext) {
        SDL_GL_DestroyContext(m_glContext);
        m_glContext = nullptr;
    }
    if (m_window) {
        SDL_DestroyWindow(m_window);
        m_window = nullptr;
    }
    m_initialized = false;
}

void Window::SwapBuffers() {
    if (m_window) SDL_GL_SwapWindow(m_window);
}

void Window::SetTitle(const std::string& title) {
    if (m_window) SDL_SetWindowTitle(m_window, title.c_str());
}

}  // namespace AIForge
