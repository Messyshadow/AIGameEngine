#pragma once

#include "../core/Math.h"

namespace AIForge {

class Texture;

/// @ai_summary 把一张大图按网格切成 N 个等大 cell,提供"按帧索引取 UV"。
/// @ai_summary 仅引用 Texture,不拥有(析构不会释放纹理)。
/// @ai_example
///   Texture tex;
///   tex.CreateFromFile("data/textures/character/robot3_idle.png");
///   SpriteSheet sheet(tex, 200, 200);   // cell 200x200,自动按图片尺寸切
///   Vec4 uv = sheet.GetFrameUV(3);      // 第 4 帧的 (u0,v0,u1,v1)
/// @ai_related Texture, SpriteAnimation, SpriteBatcher
class SpriteSheet {
public:
    SpriteSheet() = default;
    SpriteSheet(const Texture& tex, int cellW, int cellH);

    void Set(const Texture& tex, int cellW, int cellH);

    /// @ai_summary 按索引(从 0 起,行优先)取 UV 矩形。
    Vec4 GetFrameUV(int frameIndex) const;

    int  GetFrameCount() const { return m_cols * m_rows; }
    int  GetCols()       const { return m_cols; }
    int  GetRows()       const { return m_rows; }
    int  GetCellW()      const { return m_cellW; }
    int  GetCellH()      const { return m_cellH; }
    const Texture* GetTexture() const { return m_tex; }

private:
    const Texture* m_tex = nullptr;
    int            m_cellW = 0;
    int            m_cellH = 0;
    int            m_cols  = 0;
    int            m_rows  = 0;
};

}  // namespace AIForge
