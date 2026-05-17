#pragma once

#include "Framebuffer.h"
#include "Shader.h"

namespace AIForge {

/// @ai_summary 后处理管线:把"场景 FBO"和"光照 FBO"合成,再做 Bloom,输出到屏幕。
/// @ai_summary 这是 Ch 06 的核心 — 多 pass 渲染:
///   1. composite = scene * (ambient + light)
///   2. bright    = 从 composite 提取高亮像素
///   3. blur      = 对 bright 做可分离高斯模糊(ping-pong)
///   4. combine   = composite + blurredBloom,再 tonemap + gamma -> 屏幕
/// @ai_example
///   PostProcess post;
///   post.Init(1280, 720);
///   // 每帧: 先把场景画进 sceneFBO,光照画进 lightFBO,然后:
///   post.Render(sceneFBO, lightFBO, lightingOn, bloomOn, winW, winH);
/// @ai_related Framebuffer, SpriteBatcher
class PostProcess {
public:
    PostProcess() = default;
    ~PostProcess();

    PostProcess(const PostProcess&) = delete;
    PostProcess& operator=(const PostProcess&) = delete;

    /// @ai_summary 初始化:编译 4 个后处理 shader + 建内部 FBO + 全屏四边形。
    bool Init(int width, int height);

    /// @ai_summary 窗口尺寸变化时重建内部 FBO。
    void Resize(int width, int height);

    void Shutdown();

    /// @ai_summary 跑完整后处理链,最终结果绘制到默认 framebuffer(屏幕)。
    /// @ai_params sceneFBO   已渲染好的场景颜色
    /// @ai_params lightFBO   已渲染好的光照贴图(累加的光)
    /// @ai_params lightingEnabled 关掉则忽略 lightFBO,直接用场景色
    /// @ai_params bloomEnabled    关掉则跳过 Bloom
    /// @ai_params screenW/H  屏幕(窗口)尺寸,用于最后一 pass 的 viewport
    void Render(const Framebuffer& sceneFBO, const Framebuffer& lightFBO,
                bool lightingEnabled, bool bloomEnabled,
                int screenW, int screenH);

    void SetBloomThreshold(float t) { m_bloomThreshold = t; }
    void SetBlurIterations(int n)   { m_blurIterations = n; }

private:
    void DrawFullscreenQuad() const;

    Shader m_compositeShader;
    Shader m_brightShader;
    Shader m_blurShader;
    Shader m_combineShader;

    Framebuffer m_compositeFBO;
    Framebuffer m_brightFBO;
    Framebuffer m_blurFBO[2];

    unsigned int m_quadVAO = 0;
    unsigned int m_quadVBO = 0;

    int   m_width          = 0;
    int   m_height         = 0;
    float m_bloomThreshold = 0.55f;
    int   m_blurIterations = 3;
};

}  // namespace AIForge
