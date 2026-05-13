#pragma once

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

#include "CommandParser.h"

namespace AIForge {

/// @ai_summary 命令处理结果：成功 / 失败 + 可选输出文本。
/// @ai_summary 失败时 errMsg 必须填写"为什么失败 + 怎么改"以便 AI 自我修正。
struct CommandResult {
    bool ok = false;
    std::string output;  // 命令的标准输出（如 list/help/get 命令）
    std::string errMsg;  // 失败信息（仅在 ok==false 时有意义）

    static CommandResult Ok(std::string out = {}) {
        return {true, std::move(out), {}};
    }
    static CommandResult Fail(std::string err) { return {false, {}, std::move(err)}; }
};

/// @ai_summary 单条命令处理函数。
using CommandHandler = std::function<CommandResult(const ParsedCommand&)>;

/// @ai_summary 命令注册表：把动词映射到处理函数；用于驱动整个引擎的文本接口。
/// @ai_summary App 内置注册了 spawn/destroy/list/set/get/export/help。
/// @ai_example
///   app.GetCommands()->Register("greet",
///       "say hello",
///       "greet <name>",
///       [](const ParsedCommand& c) {
///           if (c.tokens.empty()) return CommandResult::Fail("usage: greet <name>");
///           return CommandResult::Ok("hello " + c.tokens[0]);
///       });
/// @ai_related App::Execute, CommandParser
class CommandRegistry {
public:
    struct Entry {
        std::string verb;
        std::string description;
        std::string usage;
        CommandHandler handler;
    };

    CommandRegistry();
    ~CommandRegistry();

    /// @ai_summary 注册一条命令。重名会覆盖，并打印警告。
    /// @ai_params verb 动词（小写）；解析时会转小写匹配
    /// @ai_params description 一句话说明（用于 help / AIContext 导出）
    /// @ai_params usage 用法示例（如 "spawn <name> [at x,y,z]"）
    /// @ai_params handler 处理函数
    void Register(const std::string& verb, const std::string& description,
                  const std::string& usage, CommandHandler handler);

    /// @ai_summary 解析并执行一条命令文本。
    CommandResult Execute(const std::string& cmd);

    /// @ai_summary 列出所有已注册命令（按动词字母序）。
    std::vector<const Entry*> List() const;

    /// @ai_summary 查找一条命令；不存在返回 nullptr。
    const Entry* Find(const std::string& verb) const;

    /// @ai_summary 找最接近的已注册命令(Levenshtein 距离 <= maxDist),
    /// 用于在 Execute 失败时给 AI 友好提示("did you mean: 'spawn'?")。
    /// 距离更远则返回空字符串。
    /// @ai_example registry.FindNearest("spwan") -> "spawn"
    std::string FindNearest(const std::string& verb, int maxDist = 2) const;

private:
    std::unordered_map<std::string, Entry> m_entries;
};

}  // namespace AIForge
