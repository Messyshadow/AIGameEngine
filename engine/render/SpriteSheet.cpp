#include "SpriteSheet.h"

#include "Texture.h"

namespace AIForge {

SpriteSheet::SpriteSheet(const Texture& tex, int cellW, int cellH) {
    Set(tex, cellW, cellH);
}

void SpriteSheet::Set(const Texture& tex, int cellW, int cellH) {
    m_tex   = &tex;
    m_cellW = cellW > 0 ? cellW : 1;
    m_cellH = cellH > 0 ? cellH : 1;
    m_cols  = tex.GetWidth()  / m_cellW;
    m_rows  = tex.GetHeight() / m_cellH;
}

Vec4 SpriteSheet::GetFrameUV(int frameIndex) const {
    if (!m_tex || m_cols == 0 || m_rows == 0) return {0, 0, 1, 1};
    int total = m_cols * m_rows;
    if (frameIndex < 0)         frameIndex = 0;
    if (frameIndex >= total)    frameIndex = total - 1;

    int col = frameIndex % m_cols;
    int row = frameIndex / m_cols;
    float tw = (float)m_tex->GetWidth();
    float th = (float)m_tex->GetHeight();
    float u0 = (col * m_cellW)       / tw;
    float u1 = ((col + 1) * m_cellW) / tw;
    float v0 = (row * m_cellH)       / th;
    float v1 = ((row + 1) * m_cellH) / th;
    // 半像素内缩,避免相邻 cell 渗色
    float pxU = 0.5f / tw;
    float pxV = 0.5f / th;
    return {u0 + pxU, v0 + pxV, u1 - pxU, v1 - pxV};
}

}  // namespace AIForge
