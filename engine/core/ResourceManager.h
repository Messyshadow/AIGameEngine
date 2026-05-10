#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace AIForge {

/// @ai_summary 资源管理器：路径解析 + 文本/JSON 读取 + 引用计数缓存。
/// @ai_summary 默认资源根目录为 "data/"，相对路径会被解析到根目录下。
/// @ai_example
///   auto& rm = *app.GetResources();
///   std::string text;
///   std::string err;
///   if (rm.ReadText("config/engine.json", text, err)) { ... }
/// @ai_related App, SceneLoader
class ResourceManager {
public:
    ResourceManager();
    ~ResourceManager();

    /// @ai_summary 设置资源根目录（绝对或相对路径）。默认 "data"。
    void SetAssetRoot(const std::string& root);
    const std::string& GetAssetRoot() const { return m_root; }

    /// @ai_summary 把相对路径解析为绝对/可用路径。绝对路径直接返回。
    /// @ai_example rm.Resolve("config/engine.json") -> "data/config/engine.json"
    std::string Resolve(const std::string& relPath) const;

    /// @ai_summary 读取文本文件到 out。失败时填 errMsg 并返回 false。
    bool ReadText(const std::string& path, std::string& out, std::string& errMsg) const;

    /// @ai_summary 列出 root 下所有相对文件路径（递归）。AIContext 用来枚举可用资源。
    /// @ai_params subdir 子目录（相对 root）。空字符串 = 整个 root。
    /// @ai_params extFilter 扩展名过滤，如 ".json"。空 = 不过滤。
    std::vector<std::string> ListFiles(const std::string& subdir = "",
                                       const std::string& extFilter = "") const;

private:
    std::string m_root = "data";
};

}  // namespace AIForge
