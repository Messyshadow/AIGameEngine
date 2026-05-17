#pragma once

#include <cstdint>
#include <string>

namespace AIForge {

/// @ai_summary OpenGL 2D 纹理封装。Ch 04 仅支持从 raw RGBA 字节数据创建。
/// @ai_summary Ch 05 起会加 stb_image 从 PNG/JPG 加载。
/// @ai_example
///   uint8_t pixels[64 * 64 * 4] = { ... };
///   Texture tex;
///   tex.CreateFromRGBA(64, 64, pixels);
///   tex.Bind(0);
///   shader.SetInt("u_Texture", 0);
/// @ai_related Shader, SpriteBatcher
class Texture {
public:
    Texture();
    ~Texture();

    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;

    /// @ai_summary 用一段 RGBA(每像素 4 字节)数据创建 GL 纹理。
    /// @ai_params width   纹理宽
    /// @ai_params height  纹理高
    /// @ai_params pixels  指向 width * height * 4 字节的 RGBA 数据
    /// @ai_params useMipmap   是否生成 mipmap(默认 false)
    /// @ai_params linearFilter 是否用线性过滤(默认 true;像素艺术请关掉)
    bool CreateFromRGBA(int width, int height, const uint8_t* pixels,
                        bool useMipmap = false, bool linearFilter = true);

    /// @ai_summary 从 PNG/JPG/BMP 文件加载(stb_image)。文件路径可以是相对工作目录或绝对。
    /// @ai_summary 加载后会以 RGBA8 上传到 GPU,自动垂直翻转(GL 习惯)。
    /// @ai_example
    ///   Texture tex;
    ///   tex.CreateFromFile("data/textures/character/robot3_idle.png");
    bool CreateFromFile(const std::string& path,
                        bool useMipmap = false, bool linearFilter = true);

    void Destroy();

    /// @ai_summary 把纹理绑定到指定 texture unit(对应 shader 里 sampler2D 的 layout)。
    void Bind(int unit = 0) const;

    int  GetWidth()  const { return m_width; }
    int  GetHeight() const { return m_height; }
    unsigned int GetID() const { return m_id; }
    bool IsValid()   const { return m_id != 0; }

private:
    unsigned int m_id     = 0;
    int          m_width  = 0;
    int          m_height = 0;
};

}  // namespace AIForge
