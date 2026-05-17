#pragma once

#include <vector>

#include "../core/Math.h"

namespace AIForge {

class Camera2D;
class Shader;
class Texture;

/// @ai_summary 2D 批渲染器 — 把上千个 sprite 合并到单个 draw call。
/// @ai_summary 工作流:Init() 一次性建好 VBO/VAO/Shader → 每帧 Begin/Submit×N/End。
/// @ai_summary 本章版本是【单纹理批渲染】:Begin 时绑定一张纹理,所有 Submit 共用。
/// @ai_summary 多纹理 atlas 批渲染在 Ch 05 加。
/// @ai_example
///   SpriteBatcher batcher;
///   batcher.Init();
///   Texture tex;  tex.CreateFromRGBA(64, 64, pixels);
///
///   while (running) {
///       batcher.Begin(camera, tex);
///       for (auto& s : sprites)
///           batcher.Submit(s.pos, s.size, s.color);
///       batcher.End();   // ← 1 次 draw call 画完
///       printf("draw calls: %d\n", batcher.GetDrawCallCount());
///   }
/// @ai_related Camera2D, Shader, Texture
class SpriteBatcher {
public:
    /// @ai_summary 混合模式:Alpha = 普通透明(默认);Additive = 加法混合(发光/粒子/光照)。
    enum class BlendMode { Alpha, Additive };

    SpriteBatcher();
    ~SpriteBatcher();

    SpriteBatcher(const SpriteBatcher&) = delete;
    SpriteBatcher& operator=(const SpriteBatcher&) = delete;

    /// @ai_summary 初始化:创建 VBO/EBO/VAO + 编译内置 sprite shader。
    /// @ai_params maxSprites 一批最多多少 sprite(超过会自动分批)。默认 10000。
    bool Init(int maxSprites = 10000);

    void Shutdown();

    /// @ai_summary 开始新的一帧批次。会重置统计计数器。
    /// @ai_params cam   当前摄像机(其 ViewProjection 将传给 shader)
    /// @ai_params tex   本批次绑定的纹理(单纹理模式)
    /// @ai_params blend 混合模式,默认 Alpha;画光源/发光粒子用 Additive
    void Begin(const Camera2D& cam, const Texture& tex,
               BlendMode blend = BlendMode::Alpha);

    /// @ai_summary 提交一个 sprite。坐标 (0,0) 是世界中心,size 是宽高(像素)。
    /// @ai_params pos      世界坐标(像素)
    /// @ai_params size     宽高(像素)
    /// @ai_params color    RGBA,与纹理相乘
    /// @ai_params rotation 弧度,绕 sprite 中心
    void Submit(Vec2 pos, Vec2 size, Vec4 color, float rotation = 0.0f);

    /// @ai_summary 提交一个 sprite,带 UV 子矩形(用于 sprite sheet 取帧)。
    /// @ai_params uvRect (u0, v0, u1, v1) — 纹理坐标矩形,默认 {0,0,1,1} 整张图。
    /// @ai_example
    ///   Vec4 uv = sheet.GetFrameUV(currentFrame);
    ///   batcher.SubmitUV(pos, size, color, 0, uv);
    void SubmitUV(Vec2 pos, Vec2 size, Vec4 color, float rotation,
                  const Vec4& uvRect);

    /// @ai_summary 结束本批次,把所有 Submit 过的 sprite 一次性画出来。
    void End();

    // 统计接口(供 Demo 显示)
    int GetDrawCallCount() const { return m_drawCalls; }
    int GetSpriteCount()   const { return m_spritesSubmitted; }

private:
    void Flush();   // 真正干活的内部函数

    struct Vertex {
        float x, y;   // 位置(世界坐标,2D 在 Z=0)
        float u, v;   // 纹理坐标
        float r, g, b, a;  // 颜色
    };

    Shader*  m_shader  = nullptr;  // 自己拥有(unique_ptr 简化语义,用裸指针)
    unsigned int m_vao = 0;
    unsigned int m_vbo = 0;
    unsigned int m_ebo = 0;

    int  m_maxSprites      = 0;
    int  m_spritesInBatch  = 0;  // 当前批次累积
    std::vector<Vertex> m_cpuBuffer;

    Mat4 m_viewProj;
    const Texture* m_currentTex = nullptr;
    BlendMode m_blendMode = BlendMode::Alpha;

    int m_drawCalls         = 0;
    int m_spritesSubmitted  = 0;
};

}  // namespace AIForge
