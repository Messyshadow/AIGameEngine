#pragma once

namespace AIForge {

/// @ai_summary 离屏渲染目标(Framebuffer Object)。把渲染结果画进一张纹理而非屏幕。
/// @ai_summary 这是后处理(Bloom/光照合成)的基础:先画到 FBO,再把 FBO 当纹理做特效。
/// @ai_example
///   Framebuffer fb;
///   fb.Create(1280, 720);
///   fb.Bind();                 // 之后所有绘制进入 fb 的纹理
///   // ... 画场景 ...
///   Framebuffer::BindDefault(winW, winH);   // 回到屏幕
///   fb.BindColorTexture(0);    // 把结果当纹理用
/// @ai_related PostProcess, Texture, SpriteBatcher
class Framebuffer {
public:
    Framebuffer() = default;
    ~Framebuffer();

    Framebuffer(const Framebuffer&) = delete;
    Framebuffer& operator=(const Framebuffer&) = delete;

    /// @ai_summary 创建 FBO + 一张 RGBA 颜色纹理附件。失败返回 false。
    bool Create(int width, int height);

    /// @ai_summary 重新分配为新尺寸(窗口 resize 时调用)。
    void Resize(int width, int height);

    void Destroy();

    /// @ai_summary 绑定为当前渲染目标,并把 viewport 设为 FBO 尺寸。
    void Bind() const;

    /// @ai_summary 解绑回默认 framebuffer(屏幕),并设置 viewport。
    static void BindDefault(int screenWidth, int screenHeight);

    /// @ai_summary 把本 FBO 的颜色纹理绑定到指定 texture unit。
    void BindColorTexture(int unit = 0) const;

    int          GetWidth()       const { return m_width; }
    int          GetHeight()      const { return m_height; }
    unsigned int GetColorTexture()const { return m_colorTex; }
    bool         IsValid()        const { return m_fbo != 0; }

private:
    unsigned int m_fbo      = 0;
    unsigned int m_colorTex = 0;
    int          m_width    = 0;
    int          m_height   = 0;
};

}  // namespace AIForge
