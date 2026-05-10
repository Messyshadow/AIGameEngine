#pragma once

#include <string>

namespace AIForge {

class App;

/// @ai_summary AI 上下文导出器：把当前项目状态打包成给 AI 阅读的 markdown。
/// @ai_summary 内容包含：实体列表（含组件）、可用命令列表（含 usage）、可用资源列表。
/// @ai_summary 这是 AIForge 的核心创新之一：AI 不需要读源代码就能理解项目状态。
/// @ai_example
///   std::string ctx = AIForge::AIContext::Export(*app);
///   // 把 ctx 喂给任意大模型作为上下文
/// @ai_related App, World, CommandRegistry
class AIContext {
public:
    /// @ai_summary 导出当前 App 状态为 markdown 字符串。
    static std::string Export(App& app);

    /// @ai_summary 把导出结果写入文件（路径相对工作目录）。
    /// @ai_params path 输出文件路径；目录不存在会失败。
    /// @ai_params app 要导出的 App
    /// @ai_params errMsg 失败信息
    static bool ExportToFile(const std::string& path, App& app,
                             std::string& errMsg);
};

}  // namespace AIForge
