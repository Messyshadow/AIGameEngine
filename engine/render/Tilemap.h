#pragma once

#include <cstdint>
#include <vector>

#include "../core/Math.h"

namespace AIForge {

class Camera2D;
class SpriteBatcher;

/// @ai_summary 数据驱动的瓦片地图:M×N 网格,每个 cell 一个瓦片类型 ID。
/// @ai_summary 本章渲染时给每种瓦片一个颜色(程序化);Ch 06 会换真实瓦片贴图。
/// @ai_summary 渲染会做视锥裁剪(只画镜头里的瓦片)。
/// @ai_example
///   Tilemap map;
///   map.Init(32, 24, 64);   // 32 列 24 行,每瓦 64 px
///   map.GenerateProcedural();
///   map.Render(batcher, camera);
/// @ai_related SpriteBatcher, Camera2D
class Tilemap {
public:
    enum TileType : uint16_t {
        T_VOID  = 0,
        T_GRASS = 1,
        T_STONE = 2,
        T_WATER = 3,
        T_SAND  = 4,
        T_PATH  = 5,
    };

    void Init(int cols, int rows, int tileSize);

    /// @ai_summary 程序化生成一张地图:草地为底 + 随机石头簇 + 中央水域
    void GenerateProcedural(unsigned seed = 20260514u);

    void SetTile(int x, int y, uint16_t type);
    uint16_t GetTile(int x, int y) const;

    /// @ai_summary 只渲染镜头视锥内的瓦片(裁剪)。
    void Render(SpriteBatcher& batcher, const Camera2D& cam) const;

    int GetCols()     const { return m_cols; }
    int GetRows()     const { return m_rows; }
    int GetTileSize() const { return m_tileSize; }

private:
    Vec4 TileColor(uint16_t t) const;

    int                  m_cols     = 0;
    int                  m_rows     = 0;
    int                  m_tileSize = 32;
    std::vector<uint16_t> m_tiles;
};

}  // namespace AIForge
