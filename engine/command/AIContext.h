#pragma once

#include <string>

namespace AIForge {

class App;

/// @ai_summary AI 上下文导出器:把当前项目状态打包成给 AI 阅读的 markdown。
/// @ai_summary 这是 AIForge 的核心创新之一:AI 不需要读源代码就能理解项目状态。
/// @ai_summary 三种使用模式:
///   1. Export(app)              — 简单全量导出,返回 markdown 字符串(向后兼容)
///   2. Export(app, opts)        — 带预算 / 选项控制,返回 ExportResult(含 token 数)
///   3. Diff(app)                — 仅输出"上次 Export 以来的变化",大幅省 token
/// @ai_example
///   // 全量
///   std::string ctx = AIForge::AIContext::Export(*app);
///
///   // 带预算
///   AIForge::AIContext::ExportOptions opts;
///   opts.maxTokens = 1500;
///   auto r = AIForge::AIContext::Export(*app, opts);
///   printf("used %d tokens, truncated=%d\n", r.estimatedTokens, r.truncated);
///
///   // 增量
///   auto d = AIForge::AIContext::Diff(*app);
/// @ai_related App, World, CommandRegistry
class AIContext {
public:
    /// @ai_summary 导出选项 — 控制 token 预算与内容裁剪。
    struct ExportOptions {
        int  maxTokens          = 4000;   ///< 软上限,超出会按优先级裁剪
        bool diffSinceLastExport = false; ///< true = 等价于 Diff()
        bool includeEntities    = true;
        bool includeCommands    = true;
        bool includeResources   = true;
        bool includeHints       = true;   ///< 末尾的 AI 操作建议段
    };

    /// @ai_summary 带 token 计数的导出结果。
    struct ExportResult {
        std::string markdown;
        int         estimatedTokens = 0;  ///< 估算 token 数(近似)
        bool        truncated       = false; ///< 是否因 budget 被裁剪
    };

    /// @ai_summary 简单导出,返回 markdown 字符串(向后兼容 Ch 02 用法)。
    static std::string Export(App& app);

    /// @ai_summary 带选项 / 预算的导出。每次调用会更新内部"上次快照"以供 Diff 使用。
    static ExportResult Export(App& app, const ExportOptions& opts);

    /// @ai_summary 输出"上次 Export 以来的变化"(新增/删除/移动)。
    /// @ai_summary 首次调用时(没有快照)等价于一次 Export。
    /// @ai_summary 典型场景下 token 消耗为 Export 的 5-30%。
    static ExportResult Diff(App& app);

    /// @ai_summary 估算文本的 token 数(近似:英文 4 char/token,中文 1.5 char/token)。
    /// @ai_summary 不需要 100% 精确,够给开发者一个数量级感知。
    static int EstimateTokens(const std::string& text);

    /// @ai_summary 清掉内部"上次快照"。下一次 Diff 会等价于完整 Export。
    static void ResetSnapshot();

    /// @ai_summary 把简单导出结果写入文件。
    static bool ExportToFile(const std::string& path, App& app,
                             std::string& errMsg);
};

}  // namespace AIForge
