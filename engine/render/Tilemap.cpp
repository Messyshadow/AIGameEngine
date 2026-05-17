#include "Tilemap.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>

#include "Camera2D.h"
#include "SpriteBatcher.h"

namespace AIForge {

void Tilemap::Init(int cols, int rows, int tileSize) {
    m_cols     = std::max(1, cols);
    m_rows     = std::max(1, rows);
    m_tileSize = std::max(1, tileSize);
    m_tiles.assign((size_t)m_cols * m_rows, (uint16_t)T_GRASS);
}

void Tilemap::SetTile(int x, int y, uint16_t t) {
    if (x < 0 || x >= m_cols || y < 0 || y >= m_rows) return;
    m_tiles[(size_t)y * m_cols + x] = t;
}

uint16_t Tilemap::GetTile(int x, int y) const {
    if (x < 0 || x >= m_cols || y < 0 || y >= m_rows) return T_VOID;
    return m_tiles[(size_t)y * m_cols + x];
}

void Tilemap::GenerateProcedural(unsigned seed) {
    std::srand(seed);
    // 1) 全部草地
    std::fill(m_tiles.begin(), m_tiles.end(), (uint16_t)T_GRASS);

    // 2) 中央水域(椭圆形)
    float cx = m_cols * 0.5f;
    float cy = m_rows * 0.5f;
    float rx = m_cols * 0.18f;
    float ry = m_rows * 0.12f;
    for (int y = 0; y < m_rows; ++y) {
        for (int x = 0; x < m_cols; ++x) {
            float dx = (x - cx) / rx;
            float dy = (y - cy) / ry;
            if (dx * dx + dy * dy < 1.0f) SetTile(x, y, T_WATER);
        }
    }

    // 3) 水边沙滩(给水周围 1 圈换沙)
    auto copy = m_tiles;
    for (int y = 0; y < m_rows; ++y) {
        for (int x = 0; x < m_cols; ++x) {
            if (copy[(size_t)y * m_cols + x] != T_GRASS) continue;
            bool nearWater = false;
            for (int dy = -1; dy <= 1 && !nearWater; ++dy)
                for (int dx = -1; dx <= 1 && !nearWater; ++dx) {
                    int nx = x + dx, ny = y + dy;
                    if (nx >= 0 && nx < m_cols && ny >= 0 && ny < m_rows &&
                        copy[(size_t)ny * m_cols + nx] == T_WATER) {
                        nearWater = true;
                    }
                }
            if (nearWater) SetTile(x, y, T_SAND);
        }
    }

    // 4) 随机石头簇(8 个,每个 3-6 块)
    for (int c = 0; c < 8; ++c) {
        int sx = std::rand() % m_cols;
        int sy = std::rand() % m_rows;
        int blobs = 3 + std::rand() % 4;
        for (int b = 0; b < blobs; ++b) {
            int ox = sx + (std::rand() % 5) - 2;
            int oy = sy + (std::rand() % 5) - 2;
            if (GetTile(ox, oy) == T_GRASS) SetTile(ox, oy, T_STONE);
        }
    }
}

Vec4 Tilemap::TileColor(uint16_t t) const {
    switch (t) {
        case T_GRASS: return {0.25f, 0.55f, 0.25f, 1.0f};
        case T_STONE: return {0.45f, 0.45f, 0.50f, 1.0f};
        case T_WATER: return {0.15f, 0.35f, 0.65f, 1.0f};
        case T_SAND:  return {0.85f, 0.78f, 0.55f, 1.0f};
        case T_PATH:  return {0.55f, 0.42f, 0.30f, 1.0f};
        default:      return {0.0f, 0.0f, 0.0f, 1.0f};
    }
}

void Tilemap::Render(SpriteBatcher& batcher, const Camera2D& cam) const {
    // 视锥裁剪:只画镜头里的瓦片
    float halfW = (float)cam.GetViewportWidth()  * 0.5f / cam.zoom;
    float halfH = (float)cam.GetViewportHeight() * 0.5f / cam.zoom;
    float minWorldX = cam.position.x - halfW;
    float maxWorldX = cam.position.x + halfW;
    float minWorldY = cam.position.y - halfH;
    float maxWorldY = cam.position.y + halfH;

    int x0 = std::max(0,           (int)std::floor(minWorldX / m_tileSize));
    int x1 = std::min(m_cols - 1,  (int)std::floor(maxWorldX / m_tileSize));
    int y0 = std::max(0,           (int)std::floor(minWorldY / m_tileSize));
    int y1 = std::min(m_rows - 1,  (int)std::floor(maxWorldY / m_tileSize));

    for (int y = y0; y <= y1; ++y) {
        for (int x = x0; x <= x1; ++x) {
            uint16_t t = m_tiles[(size_t)y * m_cols + x];
            if (t == T_VOID) continue;
            Vec4 col = TileColor(t);
            float wx = (x + 0.5f) * m_tileSize;
            float wy = (y + 0.5f) * m_tileSize;
            batcher.Submit({wx, wy},
                           {(float)m_tileSize - 1, (float)m_tileSize - 1},
                           col);
        }
    }
}

}  // namespace AIForge
